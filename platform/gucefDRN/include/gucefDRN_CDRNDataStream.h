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

#ifndef GUCEF_DRN_CDRNSTREAMGAPINFO_H
#include "gucefDRN_CDRNStreamGapInfo.h"
#define GUCEF_DRN_CDRNSTREAMGAPINFO_H
#endif /* GUCEF_DRN_CDRNSTREAMGAPINFO_H ? */

#ifndef GUCEF_DRN_CDRNSUBSCRIBEOPTIONS_H
#include "gucefDRN_CDRNSubscribeOptions.h"
#define GUCEF_DRN_CDRNSUBSCRIBEOPTIONS_H
#endif /* GUCEF_DRN_CDRNSUBSCRIBEOPTIONS_H ? */

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
 *
 *  Sequences: every entry has a 64 bit sequence, from CDRNDataStreamSettings::firstSeq (default 1) up, and the
 *  stream has an epoch (random per publisher unless persisted). A replica keeps the sequence of every queued entry
 *  and reports entries it will never get with GapEvent (raised at the end of the tick, before EntriesAvailableEvent).
 *  A subscription can start at a sequence (CDRNSubscribeOptions, SetResumeFromSeq()): while the serving side still
 *  retains that entry, every entry from there arrives once and in order.
 *  The methods are thread-safe through the lock of the node.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataStream : public CDRNDataItem
{
    public:

    static const CORE::CEvent EntriesAvailableEvent;   /**< CORE::TCloneableUInt32: the entries queued; once per tick (not with notify POLL) */
    static const CORE::CEvent GapEvent;                /**< CDRNStreamGapEventData: entries this replica will never get; only while the application subscribed */

    static void RegisterEvents( void );

    const CDRNDataStreamSettings& GetSettings( void ) const;

    void SetNotifyMode( TDRNNotifyMode mode );

    // publisher

    /** Appends an entry; false while the backlog limit holds the publisher back */
    bool AppendEntry( const CORE::CVariantStream& segment );

    bool AppendEntry( const void* data, UInt32 dataSize );

    /** Appends an entry and gives the sequence it got */
    bool AppendEntry( const CORE::CVariantStream& segment, UInt64& seq );

    bool AppendEntry( const void* data, UInt32 dataSize, UInt64& seq );

    /** The stream to write the next entry into, then CommitEntry() */
    CORE::CVariantStream& GetWriteStream( void );

    bool CommitEntry( void );

    /** Appends the entry written into GetWriteStream() and gives the sequence it got */
    bool CommitEntry( UInt64& seq );

    /** The oldest entry a subscriber can still get here (publisher, or a replica that serves subscribers); 0 before a replica holds entries */
    UInt64 GetFirstRetainedSeq( void ) const;

    /** The sequence the next entry gets here (publisher, or a replica that serves subscribers); 0 before a replica holds entries */
    UInt64 GetNextSeq( void ) const;

    /** The publisher's epoch: the publisher's own, or on a replica the announced one (0 when the publisher announced none) */
    UInt64 GetStreamEpoch( void ) const;

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

    /** Reads (and removes) the oldest entry, with its sequence */
    bool ReadNextEntry( CORE::CVariantStream& segment, UInt64& seq );

    bool ReadNextEntry( CORE::CDynamicBuffer& entry, UInt64& seq );

    /** Reads an entry without removing it, index 0 is the oldest */
    bool PeekEntry( UInt32 index, CORE::CVariantStream& segment ) const;

    bool PeekEntry( UInt32 index, CORE::CVariantStream& segment, UInt64& seq ) const;

    /** The sequence of the next entry expected from upstream, 0 before anything arrived */
    UInt64 GetNextExpectedSeq( void ) const;

    /**
     *  The next (re)subscription of this replica asks for the entries from seq (0 = the retained history).
     *  When already subscribed, the unread entries are dropped and the replica subscribes again now.
     */
    void SetResumeFromSeq( UInt64 seq );

    /** Gaps since the replica was made */
    UInt64 GetGapCount( void ) const;

    /** Entries missed in gaps since the replica was made (epoch changes count none) */
    UInt64 GetMissedEntryCount( void ) const;

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
    virtual bool IsExpectedPublicData( UInt8 lane, UInt8 msgClass, UInt32 opcode, const CDRNWireReader& body ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsDue( UInt64 nodeNow ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Float32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Publisher: takes the epoch of the settings or makes a random one; false when there is no randomness */
    bool InitPublisher( void );

    /** The options of the application subscription, applied to the first subscription of this replica only */
    void ApplySubscribeOptions( const CDRNSubscribeOptions& options );

    private:

    class CSubscriber
    {
        public:
        UInt32 peerId;
        bool started;               /**< SNAPSHOT_BEGIN / END went out */
        UInt64 fromSeq;             /**< asked for, 0 = the retained history */
        UInt64 nextSendSeq;
        UInt64 creditSeq;           /**< entries before this one may be sent */

        CSubscriber( void );
    };

    typedef GUCEF::vector< CSubscriber* > TSubscriberVector;
    typedef GUCEF::vector< CORE::CDynamicBuffer* > TBufferVector;
    typedef GUCEF::vector< UInt64 > TSeqVector;
    typedef GUCEF::vector< CDRNStreamGapInfo > TGapVector;

    CDRNDataStream( const CDRNDataStream& src );
    CDRNDataStream& operator=( const CDRNDataStream& src );

    UInt32 GetRingSize( void ) const;
    bool HasRing( void ) const;
    bool PushToRing( const void* data, UInt32 dataSize );
    void TrimRing( void );
    UInt64 GetSendFrom( const CSubscriber& subscriber ) const;
    void StartSubscriber( CSubscriber& subscriber );
    bool SendEntries( CSubscriber& subscriber );
    void OnEntries( CDRNWireReader& reader );
    void OnSnapshotBegin( CDRNWireReader& reader );
    void PushToQueue( const UInt8* data, UInt32 dataSize, UInt64 seq );
    void ReportGap( UInt64 fromSeq, UInt64 toSeq, TDRNStreamGapReason reason );
    bool IsOtherEpoch( UInt64 epoch ) const;
    UInt64 GetCreditSeq( void ) const;
    CSubscriber* FindSubscriber( UInt32 peerId ) const;
    static void ToVariantStream( const CORE::CDynamicBuffer& entry, CORE::CVariantStream& segment );

    CDRNDataStreamSettings m_settings;

    // the retained entries: on the publisher and on a replica that serves subscribers
    TBufferVector m_ring;           /**< entry seq % ring size */
    bool m_ringStarted;
    UInt64 m_firstSeq;              /**< oldest entry kept */
    UInt64 m_nextSeq;               /**< seq of the next entry */
    UInt64 m_epoch;                 /**< the publisher's epoch, 0 = none announced */
    TSubscriberVector m_subscribers;
    CORE::CVariantStream m_writeStream;
    UInt64 m_entriesSent;

    // receiving
    TBufferVector m_queue;          /**< unread entries, a ring */
    TSeqVector m_queueSeqs;         /**< the sequence of each entry of m_queue, same index */
    UInt32 m_queueHead;
    UInt32 m_queueCount;
    bool m_anyReceived;
    UInt64 m_nextExpected;          /**< seq of the next entry from upstream */
    UInt64 m_snapshotEndSeq;
    bool m_snapshotKnown;
    bool m_snapshotJustCompleted;
    bool m_awaitingSnapshotBegin;   /**< a SUBSCRIBE went out: entries of an earlier subscription are not taken */
    UInt64 m_lastCreditSent;
    bool m_creditSent;
    bool m_arrived;                 /**< entries arrived this tick */
    UInt64 m_entriesReceived;
    UInt64 m_requestedFromSeq;      /**< the fromSeq of the SUBSCRIBE that is answered next */
    UInt64 m_resumeFromSeq;         /**< asked for by the application, until a SNAPSHOT_BEGIN answers it */
    UInt64 m_expectedEpoch;         /**< the epoch m_resumeFromSeq belongs to, 0 = any */
    bool m_optionsApplied;
    UInt64 m_gapCount;
    UInt64 m_missedEntryCount;
    TGapVector m_pendingGaps;       /**< raised at the end of the tick */
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
