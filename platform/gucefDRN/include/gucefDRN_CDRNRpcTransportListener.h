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

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Accepts the RPC channels the members of one DRN session open to this member ("drn" listener).
 *  Listen( "drn://<session name>" ) with the session registered in CDRNGlobal::GetSessionRegistry(), or
 *  Listen( CORE::CUri() ) on a listener made for a session directly. One listener per session takes the channels.
 *
 *  TransportAcceptedEvent is raised on the node thread with a connected transport when a member opens a channel
 *  (DRN_RPC_OPCODE_OPEN, sent by CDRNRpcTransport::Connect()), before any frame of it: the observer (a
 *  COMCORE::CRpcHost) takes the transport and its first frame, the HELLO, follows. Channels over a link without
 *  protection (requireProtectedLink) and channels beyond maxSessionsPerMember are refused, and so are channels nobody
 *  takes in the event: the member's transport disconnects.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRpcTransportListener : public COMCORE::CIRpcTransportListener
{
    public:

    /** For the listener factory: Listen() names the session */
    CDRNRpcTransportListener( void );

    /** A listener on a session the caller holds; Listen( CORE::CUri() ) starts it */
    explicit CDRNRpcTransportListener( CDRNSession& session, const CDRNRpcTransportSettings& settings = CDRNRpcTransportSettings() );

    virtual ~CDRNRpcTransportListener() GUCEF_VIRTUAL_OVERRIDE;

    /** Before Listen(): the settings of the accepted transports and the listener's limits */
    void SetSettings( const CDRNRpcTransportSettings& settings );

    const CDRNRpcTransportSettings& GetSettings( void ) const;

    /** Channels refused: unprotected link, too many per member, nobody took the transport */
    UInt32 GetRefusedCount( void ) const;

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Listen( const CORE::CUri& local ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CUri GetListenUri( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The node's thread: always false */
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNRpcSessionComponent;

    // called by the component on the node thread, node lock held
    bool OfferChannel( CDRNRpcSessionComponent& component, const CDRNPeerLinkPtr& link, UInt32 peerId, UInt32 channel );
    void OnComponentGone( void );

    CDRNRpcTransportListener( const CDRNRpcTransportListener& src );                  /**< not implemented */
    CDRNRpcTransportListener& operator=( const CDRNRpcTransportListener& src );       /**< not implemented */

    mutable MT::CMutex m_lock;          /**< leaf: the session, node and component pointers and the counters */
    CDRNRpcTransportSettings m_settings;
    CDRNSession* m_session;
    CDRNNode* m_node;
    CDRNRpcSessionComponent* m_component;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CUri m_listenUri;
    bool m_listening;
    UInt32 m_refused;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */
