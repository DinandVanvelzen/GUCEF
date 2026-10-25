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

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNELLISTENER_H
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNELLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

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

class CEosP2PChannel;

/**
 *  Accepts "eos-p2p" channels: it listens on one socket id of the P2P interface for the local user. A peer is accepted
 *  only when the socket lets it connect, which by default is nobody (see CEosP2PEndpointInfo): the host of a
 *  lobby game puts the members of the lobby on the allow list of the socket (CEosP2PSocket::SetAllowedRemotes()) so that
 *  only they can reach it, and the connection limit of the socket bounds how many do. The first packet of a peer that
 *  was let in makes the channel, which is announced with ChannelAcceptedEvent; the DRN protocol, its handshake and its
 *  authentication run over it as over any channel.
 */
class GUCEF_DRNPLUGIN_EOSP2P_EXPORT_CPP CEosP2PChannelListener : public DRN::CIDRNChannelListener
{
    public:

    CEosP2PChannelListener( void );

    virtual ~CEosP2PChannelListener();

    virtual bool Listen( const DRN::CDRNEndpoint& local ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual DRN::CDRNEndpoint GetListenEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The socket the listener listens on, for the allow list; NULL while it does not listen */
    PLUGINGLUE::EOS::CEosP2PSocket* GetSocket( void ) const;

    UInt32 GetChannelCount( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CEosP2PChannelListener > TEventCallback;

    friend class CEosP2PChannel;

    CEosP2PChannelListener( const CEosP2PChannelListener& src );
    CEosP2PChannelListener& operator=( const CEosP2PChannelListener& src );

    void OnChannelClosed( CEosP2PChannel* channel );
    Int32 FindChannel( const char* remoteUser ) const;
    void OnPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet );
    void AcceptChannel( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet );
    void Release( void );

    void OnSocketPacketReceived( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );

    void OnSocketConnectionEstablished( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData );

    void OnSocketConnectionClosed( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    private:

    struct SEntry
    {
        char remoteUser[ PLUGINGLUE::EOS::EosP2PUserIdBufferSize ];
        CEosP2PChannel* channel;
    };
    typedef GUCEF::vector< SEntry > TEntryVector;

    PLUGINGLUE::EOS::CEosPlatform* m_platform;
    PLUGINGLUE::EOS::CEosP2PSocket* m_socket;
    PLUGINGLUE::EOS::CEosP2PSocket::TAcceptPolicy m_previousPolicy;
    bool m_changedPolicy;
    UInt32 m_tickDemand;
    CEosP2PEndpointInfo m_info;
    DRN::CDRNEndpoint m_listenEndpoint;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    TEntryVector m_channels;
    bool m_listening;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< DRN::CIDRNChannelListener, CEosP2PChannelListener, MT::CMutex >    TEosP2PChannelListenerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNELLISTENER_H ? */
