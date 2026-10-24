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

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#define GUCEF_DRN_CDRNREPLICATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#include "gucefDRN_CIDRNSessionComponent.h"
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNDATASTREAM_H
#include "gucefDRN_CDRNDataStream.h"
#define GUCEF_DRN_CDRNDATASTREAM_H
#endif /* GUCEF_DRN_CDRNDATASTREAM_H ? */

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

/** Flags of SUBSCRIBE */
static const UInt32 DRN_SUBSCRIBE_FLAG_RESYNC = 0x01;   /**< the subscriber lost track and wants the whole state again */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;

/**
 *  The data grids and streams of a session: the session component of the REPL and STREAM message classes.
 *
 *  Items are addressed on the wire by (owner peer id, item id), every REPL and STREAM message starts with both.
 *  The table of items is indexed by both, so a message reaches its item without a lookup.
 *
 *  Announcing: the owner announces a new item to the sequencer, which passes the announcement on to every member
 *  and tells joining members about every item there is. Subscribing goes upstream: to the owner when there is a
 *  direct link to it, else to the sequencer, which keeps a replica of the item and serves the subscribers from it
 *  (CLIENT_SERVER always works that way). A replica subscribes upstream while the application or another member
 *  wants it and unsubscribes when nobody does, so subscriptions are cheap to change.
 *
 *  Every tick the items that are due send, the most starved first (priority accumulators) within the byte budget
 *  of each link; then the received state is handed to the application.
 *  Owned by its session; calls take the node lock.
 */
class GUCEF_DRN_EXPORT_CPP CDRNReplicator : public CIDRNSessionComponent
{
    public:

    explicit CDRNReplicator( CDRNSession& session );

    virtual ~CDRNReplicator();

    /** Publishes a grid of the local member; HOST and PROPOSAL_VOTE grids only on the sequencer */
    CDRNDataGrid* CreateGrid( const CDRNDataGridSettings& settings );

    /** Publishes a stream of the local member */
    CDRNDataStream* CreateStream( const CDRNDataStreamSettings& settings );

    /** Withdraws an item of the local member; the object stays valid (flagged removed) until the session goes */
    bool RemoveItem( CDRNDataItem* item );

    /**
     *  The application wants the items of this name, of one owner or (ownerPeerId 0) of every owner, also those
     *  announced later. Their replicas fill after SnapshotCompleteEvent.
     */
    bool Subscribe( const CORE::CString& itemName, UInt32 ownerPeerId = 0 );

    bool Unsubscribe( const CORE::CString& itemName, UInt32 ownerPeerId = 0 );

    /** Subscribes the application to one announced item */
    bool Subscribe( CDRNDataItem& item );

    bool Unsubscribe( CDRNDataItem& item );

    /** An item by name, the first one found when ownerPeerId is 0 */
    CDRNDataItem* FindItem( const CORE::CString& itemName, UInt32 ownerPeerId = 0 ) const;
    CDRNDataGrid* FindGrid( const CORE::CString& itemName, UInt32 ownerPeerId = 0 ) const;
    CDRNDataStream* FindStream( const CORE::CString& itemName, UInt32 ownerPeerId = 0 ) const;

    CDRNDataItem* GetItem( UInt32 ownerPeerId, UInt32 itemId ) const;

    /** The items known here: published locally or announced */
    void GetItems( TDRNItemInfoVector& items ) const;

    CDRNSession& GetSession( void ) const;

    // component interface, called by the session

    virtual UInt8 GetMessageClass( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionMessage( CDRNSession& session, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionTick( CDRNSession& session, UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnMemberJoined( CDRNSession& session, UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnMemberLeft( CDRNSession& session, UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionClosed( CDRNSession& session ) GUCEF_VIRTUAL_OVERRIDE;

    /** The component of the STREAM message class */
    CIDRNSessionComponent* GetStreamComponent( void );

    /** A new run of the session (Host / Join): the items of the previous run are retired */
    void Reset( void );

    // used by the items, on the node's thread

    CDRNPeerLink* GetLinkToPeer( UInt32 peerId ) const;
    UInt32 GetLocalPeerId( void ) const;
    bool IsSequencer( void ) const;

    /** The session time at the start of this tick */
    UInt64 GetTickSessionTime( void ) const;

    /** The largest message to a member that fits in one datagram */
    UInt32 GetMessageBudget( UInt32 peerId ) const;

    /** What is left of the byte budget of this tick for the link with a member */
    UInt32 GetLinkBudget( UInt32 peerId ) const;
    void ConsumeLinkBudget( UInt32 peerId, UInt32 bytes );

    /** The replica lost track: subscribe again for the whole state */
    void Resubscribe( CDRNDataItem& item );

    private:

    class CStreamComponent : public CIDRNSessionComponent
    {
        public:
        CDRNReplicator* m_replicator;
        CStreamComponent( void ) : CIDRNSessionComponent(), m_replicator( GUCEF_NULL ) {}
        virtual UInt8 GetMessageClass( void ) const GUCEF_VIRTUAL_OVERRIDE;
        virtual void OnSessionMessage( CDRNSession& session, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message ) GUCEF_VIRTUAL_OVERRIDE;
        virtual void OnSessionTick( CDRNSession& session, UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    };

    class CAppSubscription
    {
        public:
        CORE::CString name;
        UInt32 ownerPeerId;
    };

    typedef GUCEF::vector< CDRNDataItem* > TItemVector;
    typedef GUCEF::vector< TItemVector > TItemTable;
    typedef GUCEF::vector< CAppSubscription > TAppSubscriptionVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    CDRNReplicator( const CDRNReplicator& src );
    CDRNReplicator& operator=( const CDRNReplicator& src );

    void HandleMessage( UInt32 fromPeerId, const CDRNLinkMessage& message );
    void HandleAnnounce( UInt32 fromPeerId, UInt32 ownerPeerId, UInt32 itemId, CDRNWireReader& reader );
    void HandleItemRemoved( UInt32 fromPeerId, UInt32 ownerPeerId, UInt32 itemId );
    void HandleSubscribe( UInt32 fromPeerId, CDRNDataItem* item, CDRNWireReader& reader );
    void HandleUnsubscribe( UInt32 fromPeerId, CDRNDataItem* item );

    bool AddItem( CDRNDataItem* item );
    void SendAnnounce( UInt32 toPeerId, const CDRNDataItem& item );
    void AnnounceToOthers( const CDRNDataItem& item, UInt32 exceptPeerId );
    void SendItemRemoved( UInt32 toPeerId, UInt32 ownerPeerId, UInt32 itemId );
    void SendItemReference( UInt32 toPeerId, UInt32 opcode, const CDRNDataItem& item );
    void MarkRemoved( CDRNDataItem& item );
    void UpdateUpstream( CDRNDataItem& item );
    void SendSubscribe( CDRNDataItem& item, UInt32 flags );
    UInt32 ChooseUpstream( UInt32 ownerPeerId ) const;
    bool MatchesAppSubscription( const CDRNDataItem& item ) const;
    void EmitItemEvent( const CORE::CEvent& eventid, const CDRNDataItem& item );
    void RemoveSubscriberEverywhere( UInt32 peerId );
    void RetireAll( void );

    CDRNSession* m_session;
    CStreamComponent m_streamComponent;
    TItemTable m_items;                         /**< [owner peer id][item id] */
    TItemVector m_itemList;                     /**< every item, for the ticks */
    TItemVector m_retired;                      /**< items of earlier runs of the session, deleted with the replicator */
    TItemVector m_dueItems;
    TAppSubscriptionVector m_appSubscriptions;
    TUInt32Vector m_linkBudgets;                /**< per peer id, this tick */
    UInt32 m_nextItemId;
    UInt64 m_tickSessionTime;
    CORE::CDynamicBuffer m_txBuffer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */
