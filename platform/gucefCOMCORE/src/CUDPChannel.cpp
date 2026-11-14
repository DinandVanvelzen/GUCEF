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

#include <assert.h>

#ifndef GUCEF_COMCORE_CUDPMASTERSOCKET_H
#include "CUDPMasterSocket.h"
#define GUCEF_COMCORE_CUDPMASTERSOCKET_H
#endif /* GUCEF_COMCORE_CUDPMASTERSOCKET_H ? */

#include "CUDPChannel.h"

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

const CORE::CEvent CUDPChannel::UDPPacketReceivedEvent = "GUCEF::COMCORE::CUDPChannel::UDPPacketReceivedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CUDPChannel::RegisterEvents( void )
{GUCEF_TRACE;
    
    UDPPacketReceivedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CUDPChannel::CUDPChannel( CUDPMasterSocket& parentSocket ,
                          const CIPv4Address& remoteAddr   )
    : m_remotePeer( remoteAddr )      ,
      m_parentSocket( &parentSocket )
{GUCEF_TRACE;

    assert( m_parentSocket != GUCEF_NULL );
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/
    
CUDPChannel::~CUDPChannel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CIPv4Address&
CUDPChannel::GetRemoteAddress( void ) const
{GUCEF_TRACE;

    return m_remotePeer;
}

/*-------------------------------------------------------------------------*/

CUDPMasterSocket&
CUDPChannel::GetParentSocket( void ) const
{GUCEF_TRACE;

    return *m_parentSocket;
}

/*-------------------------------------------------------------------------*/

Int32
CUDPChannel::SendPacket( const void* data , 
                         UInt16 datasize  )
{GUCEF_TRACE;

    // Delegate to the socket while adding our peer address
    return m_parentSocket->SendPacketTo( m_remotePeer ,
                                         data         ,
                                         datasize     );
}

/*-------------------------------------------------------------------------*/
    
bool
CUDPChannel::DoNotifyObservers( const CORE::CEvent& eventid               ,
                                CORE::CICloneable* eventData /* = NULL */ )
{GUCEF_TRACE;
    
    return NotifyObservers( eventid   ,
                            eventData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/