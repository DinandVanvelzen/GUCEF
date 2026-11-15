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

#ifndef GUCEF_COMCORE_CUDPCHANNEL_H
#define GUCEF_COMCORE_CUDPCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

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

class CUDPMasterSocket;

/*-------------------------------------------------------------------------*/

/**
 *  Aggregate component of the CUDPMasterSocket.
 *  Provides a medium to which observers can subscribe and receive
 *  per peer notification of received data.
 */
class GUCEF_COMCORE_EXPORT_CPP CUDPChannel : public CORE::CNotifier
{
    public:

    static const CORE::CEvent UDPPacketReceivedEvent;   
    
    static void RegisterEvents( void );
    
    typedef CORE::TLinkedCloneableBuffer UDPPacketReceivedEventData;
    
    public:  
    
    const CIPv4Address& GetRemoteAddress( void ) const;
    
    CUDPMasterSocket& GetParentSocket( void ) const;
    
    /**
     *      Attempts to send a UDP packet to the peer identified by the channel.
     *      Note that the data you send must fit within the payload
     *      space of a UDP packet.
     *
     *      @param data pointer to the data you wish to send
     *      @param datasize size in bytes of the data block pointed to by data
     *      @return the actual number of bytes that where sent. -1 indicates an error.
     */
    Int32 SendPacket( const void* data , 
                      UInt16 datasize  );

    private:
    friend class CUDPMasterSocket; 
        
    CUDPChannel( CUDPMasterSocket& parentSocket ,
                 const CIPv4Address& remoteAddr   );
    
    virtual ~CUDPChannel();
    
    bool DoNotifyObservers( const CORE::CEvent& eventid         ,
                            CORE::CICloneable* eventData = GUCEF_NULL );
    
    private:
    
    CUDPChannel( const CUDPChannel& src );            /**< not implemented, not possible */
    CUDPChannel& operator=( const CUDPChannel& src ); /**< not implemented, not possible */
    
    private:
    
    CIPv4Address m_remotePeer;
    CUDPMasterSocket* m_parentSocket;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CUDPCHANNEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 06-03-2007 :
        - Initial version

-----------------------------------------------------------------------------*/