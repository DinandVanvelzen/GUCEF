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

#ifndef GUCEF_DRN_CDRNRELAYSERVICE_H
#define GUCEF_DRN_CDRNRELAYSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Results of BIND */
enum TDRNRelayBindResult
{
    DRN_RELAY_BIND_OK           = 0,
    DRN_RELAY_BIND_ID_TAKEN     = 1,    /**< another link has the asked for id in the realm */
    DRN_RELAY_BIND_REFUSED      = 2,    /**< the token was not accepted */
    DRN_RELAY_BIND_NO_SERVICE   = 3     /**< the node does not relay */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNNode;

/**
 *  Passes packets between peers that cannot reach each other directly. A peer BINDs its link to the relay under an
 *  id in a realm {realm, id (0 = assign one), token}; FORWARD {to id, packet} is passed to the link bound under that
 *  id as FORWARD {from id, packet}, untouched, so the packet header of the end to end link (sequences, times, acks)
 *  stays intact. One binding per link. A cap on the bytes per second per realm keeps a realm from taking it all.
 *  Forwarding looks bindings up by link id and peer id, without maps. Run by the node (relayEnabled), on its thread.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRelayService
{
    public:

    explicit CDRNRelayService( CDRNNode& node );

    ~CDRNRelayService();

    /** Handles a RELAY message from a link, false if it is not one for the service */
    bool OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message );

    void OnLinkClosed( CDRNPeerLink* link );

    void Tick( UInt64 nodeNow );

    UInt32 GetBindingCount( void ) const;

    UInt64 GetForwardedBytes( void ) const;

    /** Bytes dropped by the cap per realm or because the addressee is not bound */
    UInt64 GetDroppedBytes( void ) const;

    private:

    class CRealm;

    class CBinding
    {
        public:
        CRealm* realm;
        UInt32 peerId;
        CDRNPeerLink* link;
        CDRNPeerLinkPtr linkPtr;

        CBinding( void );
    };

    typedef GUCEF::vector< CBinding* > TBindingVector;

    class CRealm
    {
        public:
        CORE::CString name;
        TBindingVector bindings;        /**< the fixed peer ids, by peer id */
        TBindingVector automaticBindings;   /**< the peer ids the relay handed out, by peer id less the first automatic id */
        UInt32 bindingCount;
        UInt32 nextPeerId;
        Int64 budget;                   /**< bytes it may still forward, refilled per second */
        UInt64 lastRefill;

        CRealm( void );
    };

    typedef GUCEF::map< CORE::CString, CRealm* > TRealmMap;

    CDRNRelayService( const CDRNRelayService& src );
    CDRNRelayService& operator=( const CDRNRelayService& src );

    void OnBind( CDRNPeerLink* link, CDRNWireReader& reader );
    void OnForward( CDRNPeerLink* link, const CDRNLinkMessage& message, CDRNWireReader& reader );
    void Unbind( CDRNPeerLink* link );

    /** The binding of a peer id in a realm, NULL when it is free */
    static CBinding* FindBinding( const CRealm& realm, UInt32 peerId );

    static void SetBinding( CRealm& realm, UInt32 peerId, CBinding* binding );

    /**
     *  Whether the link of a binding is gone without the relay having seen it close: closed, or silent for longer
     *  than an alive peer can be, it answers the echoes of the relay. A new bind with valid credentials replaces it
     */
    bool IsBindingStale( const CBinding& binding ) const;
    void SendBindReply( CDRNPeerLink* link, UInt32 result, UInt32 peerId, const CORE::CString& realm );
    CBinding* GetBinding( const CDRNPeerLink* link ) const;

    CDRNNode* m_node;
    TRealmMap m_realms;
    TBindingVector m_bindingsByLinkId;
    UInt32 m_bindingCount;
    UInt64 m_forwardedBytes;
    UInt64 m_droppedBytes;
    UInt8 m_header[ 8 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRELAYSERVICE_H ? */
