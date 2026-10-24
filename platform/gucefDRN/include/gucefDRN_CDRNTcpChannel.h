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

#ifndef GUCEF_DRN_CDRNTCPCHANNEL_H
#define GUCEF_DRN_CDRNTCPCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNSTREAMFRAMER_H
#include "gucefDRN_CDRNStreamFramer.h"
#define GUCEF_DRN_CDRNSTREAMFRAMER_H
#endif /* GUCEF_DRN_CDRNSTREAMFRAMER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace COMCORE {
class CTCPClientSocket;
class CTCPServerConnection;
}

namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The "tcp" channel: DRN packets framed on a TCP connection, see CDRNStreamFramer.
 *  Either a client connection it owns (Connect()) or a connection accepted by CDRNTcpChannelListener.
 *  Nagle's algorithm is switched off.
 */
class GUCEF_DRN_EXPORT_CPP CDRNTcpChannel : public CIDRNChannel
{
    public:

    CDRNTcpChannel( void );

    /** A channel for a connection accepted by a server socket */
    CDRNTcpChannel( COMCORE::CTCPServerConnection* connection, CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CDRNTcpChannel();

    virtual bool Connect( const CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendPacket( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMaxReceivePacketSize( UInt32 maxPacketSize ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNTcpChannel( const CDRNTcpChannel& src );
    CDRNTcpChannel& operator=( const CDRNTcpChannel& src );

    void OnData( const CORE::CDynamicBuffer& data );
    void OnConnectionLost( void );
    void DetachFromSocket( void );

    COMCORE::CTCPClientSocket* m_clientSocket;
    COMCORE::CTCPServerConnection* m_serverConnection;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNChannelCapabilities m_capabilities;
    CDRNEndpoint m_remote;
    CDRNStreamFramer m_framer;
    CORE::CDynamicBuffer m_txBuffer;
    CORE::CDynamicBuffer m_rxView;
    CDRNPacketEventData m_rxEventData;
    bool m_connected;
    bool m_closing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNTCPCHANNEL_H ? */
