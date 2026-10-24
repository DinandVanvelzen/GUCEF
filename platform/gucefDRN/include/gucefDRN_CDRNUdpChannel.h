/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNUDPCHANNEL_H
#define GUCEF_DRN_CDRNUDPCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace COMCORE {
class CUDPSocket;
}

namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNUdpChannelListener;

/**
 *  The "udp" channel: one DRN packet is one datagram, with a budget of 1200 bytes (endpoint parameter
 *  "maxPacketSize"). The transport loses and reorders: the link layer adds the reliability for the reliable lanes.
 *  A connecting channel owns a socket on an ephemeral port and is connected as soon as Connect() returns;
 *  an accepted channel shares the socket of its CDRNUdpChannelListener.
 *  A connecting channel answers a HELLO_RETRY of the listener: it sends its last packet again with the cookie in
 *  front (COOKIE_ECHO) and keeps the cookie in front of what it sends until a DRN packet of the listener arrives.
 */
class GUCEF_DRN_EXPORT_CPP CDRNUdpChannel : public CIDRNChannel
{
    public:

    CDRNUdpChannel( void );

    virtual ~CDRNUdpChannel();

    virtual bool Connect( const CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendPacket( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNUdpChannelListener;

    /** An accepted channel */
    CDRNUdpChannel( CDRNUdpChannelListener* listener, const COMCORE::CIPv4Address& remote, CORE::PulseGeneratorPtr pulseGenerator, UInt32 maxPacketSize );

    CDRNUdpChannel( const CDRNUdpChannel& src );
    CDRNUdpChannel& operator=( const CDRNUdpChannel& src );

    /** @return false if the channel must not be touched anymore */
    bool OnPacket( const void* data, UInt32 size );

    /** The listener goes away */
    void OnListenerGone( void );

    /** Connecting side: a HELLO_RETRY of the listener */
    void OnHelloRetry( const UInt8* data, UInt32 size );

    bool SendRaw( const void* data, UInt32 size );

    COMCORE::CUDPSocket* m_socket;
    CDRNUdpChannelListener* m_listener;
    COMCORE::CIPv4Address m_remoteAddress;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNChannelCapabilities m_capabilities;
    CDRNEndpoint m_remote;
    CORE::CDynamicBuffer m_rxView;
    CDRNPacketEventData m_rxEventData;
    bool m_connected;
    bool m_answered;                        /**< connecting side: a DRN packet of the listener arrived */
    UInt8 m_cookie[ DRN_UDP_COOKIE_SIZE ];
    bool m_haveCookie;
    CORE::CDynamicBuffer m_lastUnanswered;  /**< connecting side: the last packet sent before the listener answered */
    CORE::CDynamicBuffer m_cookieScratch;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNUDPCHANNEL_H ? */
