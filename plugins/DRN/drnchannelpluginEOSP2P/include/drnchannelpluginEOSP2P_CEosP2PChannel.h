/*
 *  drnchannelpluginEOSP2P: DRN channel plugin that moves DRN packets over the Epic Online Services P2P interface
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

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNEL_H
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#include "pluginglueEOS_CEosGlue.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H ? */

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PENDPOINTINFO_H
#include "drnchannelpluginEOSP2P_CEosP2PEndpointInfo.h"
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PENDPOINTINFO_H
#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PENDPOINTINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNPLUGIN {
namespace EOSP2P {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CEosP2PChannelListener;

/**
 *  The "eos-p2p" channel: one DRN packet is one packet of the Epic Online Services P2P interface (at most 1170
 *  bytes), sent to a Product User Id on a socket id. EOS_P2P is only the transport. DRN runs unchanged on top of it:
 *  the protocol 2 handshake, the authentication of the link and the AEAD protection of the packets all happen above the
 *  channel, and the reliability of a datagram channel is the one of the link layer of DRN (the channel reports itself
 *  as an unreliable datagram channel with the budget of the SDK).
 *
 *  A connecting channel opens the connection to the peer with its first packet. It is connected as soon as Connect()
 *  returns; when the SDK has not established the connection within the connect timeout, or closes it, the channel
 *  ends with ChannelErrorEvent and ChannelDisconnectedEvent. An accepted channel (made by CEosP2PChannelListener) is
 *  the other end of such a connection, shared with the socket of the listener.
 *
 *  The channel runs on the pulse generator of the application, where the glue ticks the platform: a node on a
 *  thread of its own is not supported (Connect() refuses a pulse generator of another thread).
 */
class GUCEF_DRNPLUGIN_EOSP2P_EXPORT_CPP CEosP2PChannel : public DRN::CIDRNChannel
{
    public:

    CEosP2PChannel( void );

    virtual ~CEosP2PChannel();

    virtual bool Connect( const DRN::CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendPacket( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const DRN::CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual DRN::CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The Product User Id of the peer */
    const CORE::CString& GetRemoteUser( void ) const;

    /** Whether the SDK established the connection; false while it is being made */
    bool IsEstablished( void ) const;

    /** How the connection runs once it is established: directly, or through the relay of Epic */
    PLUGINGLUE::EOS::EEosNetworkType GetNetworkType( void ) const;

    UInt32 GetSentPacketCount( void ) const;

    UInt32 GetReceivedPacketCount( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CEosP2PChannel > TEventCallback;

    friend class CEosP2PChannelListener;

    /** An accepted channel: the other end of a connection that arrived on the socket of a listener, which hands it its packets */
    CEosP2PChannel( CEosP2PChannelListener* listener                ,
                    PLUGINGLUE::EOS::CEosP2PSocket* socket          ,
                    const CEosP2PEndpointInfo& info                 ,
                    const CORE::CString& remoteUser                 );

    CEosP2PChannel( const CEosP2PChannel& src );
    CEosP2PChannel& operator=( const CEosP2PChannel& src );

    bool Attach( PLUGINGLUE::EOS::CEosPlatform* platform, PLUGINGLUE::EOS::CEosP2PSocket* socket );
    void Detach( void );
    bool OnPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet );
    void OnConnectionEstablished( const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data );
    void OnConnectionClosed( const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data );
    void OnTick( void );

    void OnSocketPacketReceived( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );

    void OnSocketConnectionEstablished( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData );

    void OnSocketConnectionClosed( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    void OnPlatformTick( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );
    void End( const CORE::CString* error );
    void OnListenerGone( void );
    bool IsOurs( const char* remoteUser ) const;

    private:

    CEosP2PChannelListener* m_listener;
    PLUGINGLUE::EOS::CEosPlatform* m_platform;
    PLUGINGLUE::EOS::CEosP2PSocket* m_socket;
    UInt32 m_tickDemand;
    CEosP2PEndpointInfo m_info;
    CORE::CString m_remoteUser;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    DRN::CDRNChannelCapabilities m_capabilities;
    DRN::CDRNEndpoint m_remote;
    CORE::CDynamicBuffer m_rxView;
    DRN::CDRNPacketEventData m_rxEventData;
    bool m_connected;
    bool m_established;
    UInt64 m_deadlineInMs;
    PLUGINGLUE::EOS::EEosNetworkType m_networkType;
    UInt32 m_sent;
    UInt32 m_received;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< DRN::CIDRNChannel, CEosP2PChannel, MT::CMutex >    TEosP2PChannelFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNEL_H ? */
