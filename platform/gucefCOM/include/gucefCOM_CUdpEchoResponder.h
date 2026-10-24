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

#ifndef GUCEF_COM_CUDPECHORESPONDER_H
#define GUCEF_COM_CUDPECHORESPONDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Answers the probes of CUdpEchoRttProbe: every 16 byte ['G','D','E','C'][u32 seq][u64 send time] datagram
 *  goes back to where it came from, unchanged. Anything else is left alone.
 *
 *  It runs on a socket of its own, for example on a port next to the game port of an application, or it rides
 *  along on a socket the application already has: then that socket keeps receiving everything as before and the
 *  probes are answered as well.
 */
class GUCEF_COM_PUBLIC_CPP CUdpEchoResponder : public CORE::CObservingNotifier
{
    public:

    static const UInt8 EchoMagic[ 4 ];
    static const UInt32 EchoPacketSize;     /**< 16 */

    CUdpEchoResponder( void );

    explicit CUdpEchoResponder( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CUdpEchoResponder();

    /** Opens a socket of its own on the given port, 0 lets the system pick one */
    bool Start( UInt16 port );

    /** Opens a socket of its own on the given local address and port */
    bool Start( const COMCORE::CIPv4Address& localAddress );

    /** Answers the probes that arrive on the given socket, which stays the caller's. Stop() before the socket goes. */
    bool Attach( COMCORE::CUDPSocket& socket );

    void Stop( void );

    bool IsActive( void ) const;

    /** The port of the socket in use, host byte order */
    UInt16 GetPort( void ) const;

    UInt32 GetEchoCount( void ) const;

    /** Whether the datagram is an echo probe */
    static bool IsEchoPacket( const void* data, UInt32 dataSize );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CUdpEchoResponder( const CUdpEchoResponder& src );
    CUdpEchoResponder& operator=( const CUdpEchoResponder& src );

    private:

    COMCORE::CUDPSocket m_ownSocket;
    COMCORE::CUDPSocket* m_socket;
    UInt32 m_echoCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CUDPECHORESPONDER_H ? */
