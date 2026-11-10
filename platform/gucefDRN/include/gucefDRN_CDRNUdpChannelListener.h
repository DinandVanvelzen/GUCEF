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

#ifndef GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#define GUCEF_DRN_CDRNUDPCHANNELLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

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

class CDRNUdpChannel;

/**
 *  Accepts "udp" channels: one socket, the datagrams are handed to a channel per remote address.
 *
 *  With stateless cookies (the default, CDRNSecurityPolicy::statelessCookies) nothing is kept for an unknown
 *  address: its DRN packet is answered with a HELLO_RETRY that carries a cookie, an HMAC of the address and the
 *  current time window under a secret of the listener. Only a COOKIE_ECHO with a valid cookie (of this window or
 *  the one before) creates a channel. A spoofed address never sees the cookie, so it never gets state; the retry
 *  is never larger than what it answers. Without cookies the first datagram that looks like a DRN packet creates
 *  a channel.
 */
class GUCEF_DRN_EXPORT_CPP CDRNUdpChannelListener : public CIDRNChannelListener
{
    public:

    CDRNUdpChannelListener( void );

    virtual ~CDRNUdpChannelListener();

    virtual bool Listen( const CDRNEndpoint& local ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CDRNEndpoint GetListenEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplySecurityPolicy( const CDRNSecurityPolicy& policy ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    UInt32 GetChannelCount( void ) const;

    /** Stateless cookies before any state for an address, to be set before Listen(), default true */
    void SetStatelessCookies( bool enabled );

    bool IsUsingStatelessCookies( void ) const;

    /** HELLO_RETRY datagrams sent */
    UInt32 GetCookieRetryCount( void ) const;

    /** COOKIE_ECHO datagrams of unknown addresses with a cookie that was not valid */
    UInt32 GetInvalidCookieCount( void ) const;

    /** The socket of the listener, for asking a STUN server what the port looks like from the outside */
    COMCORE::CUDPSocket* GetSocket( void ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNUdpChannel;

    typedef GUCEF::map< COMCORE::CIPv4Address, CDRNUdpChannel* > TChannelMap;

    CDRNUdpChannelListener( const CDRNUdpChannelListener& src );
    CDRNUdpChannelListener& operator=( const CDRNUdpChannelListener& src );

    bool SendTo( const COMCORE::CIPv4Address& remote, const void* data, UInt32 size );
    void OnChannelClosed( CDRNUdpChannel* channel );
    void OnDatagram( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& data );
    void AcceptChannel( const COMCORE::CIPv4Address& source, const UInt8* packet, UInt32 packetSize );
    void MakeCookieMac( const COMCORE::CIPv4Address& source, UInt32 window, UInt8* mac ) const;
    bool IsCookieValid( const COMCORE::CIPv4Address& source, const UInt8* cookie, UInt32 size ) const;
    void SendHelloRetry( const COMCORE::CIPv4Address& source );

    COMCORE::CUDPSocket* m_socket;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNEndpoint m_listenEndpoint;
    UInt32 m_maxPacketSize;
    TChannelMap m_channels;
    COMCORE::CIPv4Address m_lastSource;     /**< the channel of the last datagram, saves the lookup for bursts */
    CDRNUdpChannel* m_lastChannel;
    bool m_statelessCookies;
    UInt8 m_cookieSecret[ 32 ];
    UInt32 m_cookieRetries;
    UInt32 m_invalidCookies;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNUDPCHANNELLISTENER_H ? */
