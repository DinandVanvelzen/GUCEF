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

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#define GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBSOCKETCONNECTION_H
#include "gucefWEB_CWebSocketConnection.h"
#define GUCEF_WEB_CWEBSOCKETCONNECTION_H
#endif /* GUCEF_WEB_CWEBSOCKETCONNECTION_H ? */

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

namespace WEB {
class CWebSocketClient;
}

namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNWebSocketChannelListener;

/**
 *  The "websocket" channel: one DRN packet is one binary web socket message. Reliable and browser compatible.
 *  Endpoints are ws://host:port/path. Needs the sha1 codec (codecspluginSTBRUMMEHASH) for the web socket handshake.
 */
class GUCEF_DRN_EXPORT_CPP CDRNWebSocketChannel : public CIDRNChannel
{
    public:

    CDRNWebSocketChannel( void );

    virtual ~CDRNWebSocketChannel();

    virtual bool Connect( const CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendPacket( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The message size limit of the web socket connection, accepted or connecting; a larger message closes it */
    virtual void SetMaxReceivePacketSize( UInt32 maxPacketSize ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNWebSocketChannelListener;

    /** An accepted channel */
    CDRNWebSocketChannel( CDRNWebSocketChannelListener* listener, const WEB::TWebSocketConnectionPtr& connection, CORE::PulseGeneratorPtr pulseGenerator );

    CDRNWebSocketChannel( const CDRNWebSocketChannel& src );
    CDRNWebSocketChannel& operator=( const CDRNWebSocketChannel& src );

    bool OnPacket( const CORE::CDynamicBuffer& data );
    void OnRemoteClosed( void );
    void OnListenerGone( void );

    WEB::CWebSocketClient* m_client;
    CDRNWebSocketChannelListener* m_listener;
    WEB::TWebSocketConnectionPtr m_connection;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNChannelCapabilities m_capabilities;
    CDRNEndpoint m_remote;
    CDRNPacketEventData m_rxEventData;
    bool m_connected;
    bool m_closing;
    UInt32 m_maxReceivePacketSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H ? */
