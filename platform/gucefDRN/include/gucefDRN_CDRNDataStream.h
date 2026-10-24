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

#ifndef GUCEF_DRN_CDRNDATASTREAM_H
#define GUCEF_DRN_CDRNDATASTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_DRN_CDRNDATAITEM_H
#include "gucefDRN_CDRNDataItem.h"
#define GUCEF_DRN_CDRNDATAITEM_H
#endif /* GUCEF_DRN_CDRNDATAITEM_H ? */

#ifndef GUCEF_DRN_CDRNDATASTREAMSETTINGS_H
#include "gucefDRN_CDRNDataStreamSettings.h"
#define GUCEF_DRN_CDRNDATASTREAMSETTINGS_H
#endif /* GUCEF_DRN_CDRNDATASTREAMSETTINGS_H ? */

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
 *  A stream of entries published by one member: every entry reaches every subscriber, in order (lane 0).
 *
 *  An entry is one segment of a CORE::CVariantStream (any number of variants closed with WriteSegmentEnd()).
 *  Flow control: a receiver grants CREDIT for backlogLimit unread entries; the publisher sends no further and
 *  AppendEntry() refuses entries while the slowest subscriber has backlogLimit entries outstanding. The publisher
 *  keeps the last retainedHistory entries for late subscribers (chat, a log of events).
 *  Entry buffers are kept and reused, so a steady stream does not allocate.
 *  The methods are thread-safe through the lock of the node.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataStream : public CDRNDataItem
{
    public:

    static const CORE::CEvent EntriesAvailableEvent;   /**< CORE::TCloneableUInt32: the entries queued; once per tick (not with notify POLL) */

    static void RegisterEvents( void );

    const CDRNDataStreamSettings& GetSettings( void ) const;

    void SetNotifyMode( TDRNNotifyMode mode );

    // publisher

    /** Appends an entry; false while the backlog limit holds the publisher back */
    bool AppendEntry( const CORE::CVariantStream& segment );

    bool AppendEntry( const void* data, UInt32 dataSize );

    /** The stream to write the next entry into, then CommitEntry() */
    CORE::CVariantStream& GetWriteStream( void );

    bool CommitEntry( void );

    /** False while the slowest subscriber has backlogLimit entries outstanding */
    bool CanAppend( void ) const;

    /** Entries the slowest subscriber has not been sent yet */
    UInt32 GetBacklog( void ) const;

    // receiver

    /** Entries received and not read yet */
    UInt32 GetEntryCount( void ) const;

    /** Reads (and removes) the oldest entry */
    bool ReadNextEntry( CORE::CVariantStream& segment );

    bool ReadNextEntry( CORE::CDynamicBuffer& entry );

    /** Reads an entry without removing it, index 0 is the oldest */
    bool PeekEntry( UInt32 index, CORE::CVariantStream& segment ) const;

    /** Removes the oldest entries, returns how many */
    UInt32 DiscardEntries( UInt32 count );

    UInt64 GetEntriesSent( void ) const;
    UInt64 GetEntriesReceived( void ) const;

    /** The queued (receiver) or retained (publisher) entries; never used on the wire */
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Publisher only: appends the entries of a dump */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    friend class CDRNReplicator;

    CDRNDataStream( CDRNReplicator& replicator, const CDRNDataStreamSettings& settings );
    virtual ~CDRNDataStream();

    virtual void OnSubscriberAdded( UInt32 peerId, UInt32 flags, CDRNWireReader& params ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSubscriberRemoved( UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnItemMessage( UInt32 fromPeerId, UInt8 msgClass, UInt32 opcode, CDRNWireReader& reader ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnSendTick( UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnEndOfTick( UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void WriteAnnounceSettings( CDRNWireWriter& writer ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void ReadAnnounceSettings( CDRNWireReader& reader ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void WriteSubscribeParams( CDRNWireWriter& writer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnResubscribe( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsDue( UInt64 nodeNow ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Float32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CSubscriber
    {
        public:
        UInt32 peerId;
        bool started;               /**< SNAPSHOT_BEGIN / END went out */
        UInt32 fromSeq;             /**< asked for, 0 = the retained history */
        UInt32 nextSendSeq;
        UInt32 creditSeq;           /**< entries before this one may be sent */

        CSubscriber( void );
    };

    typedef GUCEF::vector< CSubscriber* > TSubscriberVector;
    typedef GUCEF::vector< CORE::CDynamicBuffer* > TBufferVector;

    CDRNDataStream( const CDRNDataStream& src );
    CDRNDataStream& operator=( const CDRNDataStream& src );

    UInt32 GetRingSize( void ) const;
    bool HasRing( void ) const;
    bool PushToRing( const void* data, UInt32 dataSize );
    void TrimRing( void );
    void StartSubscriber( CSubscriber& subscriber );
    bool SendEntries( CSubscriber& subscriber );
    void OnEntries( CDRNWireReader& reader );
    void PushToQueue( const UInt8* data, UInt32 dataSize );
    UInt32 GetCreditSeq( void ) const;
    CSubscriber* FindSubscriber( UInt32 peerId ) const;
    static void ToVariantStream( const CORE::CDynamicBuffer& entry, CORE::CVariantStream& segment );

    CDRNDataStreamSettings m_settings;

    // the retained entries: on the publisher and on a replica that serves subscribers
    TBufferVector m_ring;           /**< entry seq % ring size */
    bool m_ringStarted;
    UInt32 m_firstSeq;              /**< oldest entry kept */
    UInt32 m_nextSeq;               /**< seq of the next entry */
    TSubscriberVector m_subscribers;
    CORE::CVariantStream m_writeStream;
    UInt64 m_entriesSent;

    // receiving
    TBufferVector m_queue;          /**< unread entries, a ring */
    UInt32 m_queueHead;
    UInt32 m_queueCount;
    bool m_anyReceived;
    UInt32 m_nextExpected;          /**< seq of the next entry from upstream */
    UInt32 m_snapshotEndSeq;
    bool m_snapshotKnown;
    bool m_snapshotJustCompleted;
    UInt32 m_lastCreditSent;
    bool m_creditSent;
    bool m_arrived;                 /**< entries arrived this tick */
    UInt64 m_entriesReceived;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATASTREAM_H ? */
