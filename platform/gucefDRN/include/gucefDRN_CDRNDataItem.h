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

#ifndef GUCEF_DRN_CDRNDATAITEM_H
#define GUCEF_DRN_CDRNDATAITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#include "gucefDRN_CDRNDataGridSettings.h"
#define GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#endif /* GUCEF_DRN_CDRNDATAGRIDSETTINGS_H ? */

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

class CDRNReplicator;
class CDRNSession;

/**
 *  An item that a session replicates: a data grid or a data stream.
 *
 *  An item is published by the member that created it (its owner, the origin) and replicated to the members that
 *  subscribe to it. In CLIENT_SERVER the server keeps a replica of every item a client publishes and passes it on
 *  to the other subscribers. Items are addressed on the wire by (owner peer id, item id).
 *  Items are created and owned by their session; events reach the application as the session's events do.
 *  An item can be dumped to a CDataNode (Serialize) for debugging; it is bound to its session and cannot be cloned.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataItem : public CORE::CTSGNotifier          ,
                                          public CORE::CIDataNodeSerializable
{
    public:

    enum TItemKind
    {
        ITEMKIND_GRID   = 1,
        ITEMKIND_STREAM = 2
    };

    static const CORE::CEvent SnapshotCompleteEvent;   /**< a replica got the whole state of its item */
    static const CORE::CEvent ItemRemovedEvent;        /**< the publisher removed the item or left */

    static void RegisterEvents( void );

    const CORE::CString& GetName( void ) const;
    TItemKind GetKind( void ) const;
    UInt32 GetOwnerPeerId( void ) const;
    UInt32 GetItemId( void ) const;

    /** True for the item of the local member, false for a replica */
    bool IsOrigin( void ) const;

    /** A replica whose snapshot arrived */
    bool IsSnapshotComplete( void ) const;

    /** The publisher removed the item or left */
    bool IsRemoved( void ) const;

    /** Members the item is sent to from here */
    UInt32 GetSubscriberCount( void ) const;

    CDRNSession& GetSession( void ) const;

    /** Items are bound to their session: returns NULL */
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    friend class CDRNReplicator;

    CDRNDataItem( CDRNReplicator& replicator, TItemKind kind );
    virtual ~CDRNDataItem();

    // the replicator drives the item, on the node's thread

    /** A member subscribes (or asks for a resync), params has what followed the item reference in SUBSCRIBE */
    virtual void OnSubscriberAdded( UInt32 peerId, UInt32 flags, CDRNWireReader& params ) = 0;
    virtual void OnSubscriberRemoved( UInt32 peerId ) = 0;
    virtual void OnItemMessage( UInt32 fromPeerId, UInt8 msgClass, UInt32 opcode, CDRNWireReader& reader ) = 0;

    /** Sends what is due; returns false when the budget ran out and something stayed behind */
    virtual bool OnSendTick( UInt64 nodeNow ) = 0;

    /** End of the tick: hand received data to the application, acknowledge and grant credit upstream */
    virtual void OnEndOfTick( UInt64 nodeNow ) = 0;

    /** The settings that travel in ITEM_ANNOUNCE */
    virtual void WriteAnnounceSettings( CDRNWireWriter& writer ) const = 0;
    virtual void ReadAnnounceSettings( CDRNWireReader& reader ) = 0;

    /** What follows the item reference in this replica's SUBSCRIBE */
    virtual void WriteSubscribeParams( CDRNWireWriter& writer );

    /** The upstream changed or must be asked again: the replica starts over */
    virtual void OnResubscribe( void );

    /** The item went away at the publisher */
    virtual void OnRemoved( void );

    virtual bool IsDue( UInt64 nodeNow ) const = 0;
    virtual Float32 GetPriority( void ) const = 0;

    void Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData = GUCEF_NULL );

    /** Starts a message to a member: the item reference is written, the rest follows */
    CDRNWireWriter& BeginMessage( void );
    bool SendMessage( UInt32 peerId, UInt8 lane, UInt8 msgClass, UInt32 opcode );

    CDRNReplicator& GetReplicator( void ) const;

    protected:

    CDRNReplicator* m_replicator;
    TItemKind m_kind;
    CORE::CString m_name;
    UInt32 m_ownerPeerId;
    UInt32 m_itemId;
    bool m_isOrigin;
    bool m_isRelay;             /**< kept by the server for the other subscribers, not subscribed by the application */
    bool m_appSubscribed;       /**< the application asked for this replica */
    bool m_subscribeSent;
    UInt32 m_upstreamPeerId;
    bool m_snapshotComplete;
    bool m_removed;
    Float32 m_priorityAccumulator;
    UInt32 m_subscriberCount;
    CORE::CDynamicBuffer m_txBuffer;
    CDRNWireWriter m_txWriter;

    private:

    CDRNDataItem( const CDRNDataItem& src );
    CDRNDataItem& operator=( const CDRNDataItem& src );
};

/*-------------------------------------------------------------------------*/

/** Event data of the item events of a session */
class GUCEF_DRN_EXPORT_CPP CDRNItemInfo
{
    public:

    UInt32 ownerPeerId;
    UInt32 itemId;
    CORE::CString name;
    CDRNDataItem::TItemKind kind;

    CDRNItemInfo( void );
};

typedef CORE::CTCloneableObj< CDRNItemInfo >  CDRNItemInfoEventData;
typedef GUCEF::vector< CDRNItemInfo >          TDRNItemInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATAITEM_H ? */
