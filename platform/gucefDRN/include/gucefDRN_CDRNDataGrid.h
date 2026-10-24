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

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#define GUCEF_DRN_CDRNDATAGRID_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNDATAITEM_H
#include "gucefDRN_CDRNDataItem.h"
#define GUCEF_DRN_CDRNDATAITEM_H
#endif /* GUCEF_DRN_CDRNDATAITEM_H ? */

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#include "gucefDRN_CDRNVariantCodec.h"
#define GUCEF_DRN_CDRNVARIANTCODEC_H
#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */

#ifndef GUCEF_DRN_CDRNLINKRELIABILITY_H
#include "gucefDRN_CDRNLinkReliability.h"
#define GUCEF_DRN_CDRNLINKRELIABILITY_H
#endif /* GUCEF_DRN_CDRNLINKRELIABILITY_H ? */

#ifndef GUCEF_DRN_CIDRNWRITEVALIDATOR_H
#include "gucefDRN_CIDRNWriteValidator.h"
#define GUCEF_DRN_CIDRNWRITEVALIDATOR_H
#endif /* GUCEF_DRN_CIDRNWRITEVALIDATOR_H ? */

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

/** Event data of ProposeResultEvent: how a write proposed by this member went */
class GUCEF_DRN_EXPORT_CPP CDRNProposeResult
{
    public:

    UInt32 proposalId;
    bool accepted;
    UInt32 voteOutcome;         /**< CDRNVotePolicy::TVoteOutcome when the write was put to a vote, else PENDING */
    CORE::CString reason;

    CDRNProposeResult( void );
};

typedef CORE::CTCloneableObj< CDRNProposeResult > CDRNProposeResultEventData;

/*-------------------------------------------------------------------------*/

/** Event data of VoteStartedEvent and VoteEndedEvent */
class GUCEF_DRN_EXPORT_CPP CDRNVoteInfo
{
    public:

    UInt32 voteId;
    UInt32 proposerPeerId;
    CORE::CVariant key;
    CORE::CVariant value;
    bool isRemoval;             /**< the proposal removes the key */
    UInt32 timeoutMs;
    UInt32 outcome;             /**< CDRNVotePolicy::TVoteOutcome */

    CDRNVoteInfo( void );
};

typedef CORE::CTCloneableObj< CDRNVoteInfo > CDRNVoteEventData;

typedef GUCEF::vector< UInt32 >                     TDRNKeyIndexVector;
typedef CORE::CTCloneableObj< TDRNKeyIndexVector >  CDRNGridUpdatedEventData;

/*-------------------------------------------------------------------------*/

/**
 *  A grid of CVariant keys with one CVariant value each, replicated last one wins: the newest value counts and
 *  older ones may be dropped on the way.
 *
 *  Every key is mapped once to a dense index; KEY_DEFINE travels once per subscriber and deltas carry indices.
 *  Values live in a flat array. Per subscriber the publisher knows, per key, which change it sent and which change
 *  the subscriber acknowledged: a delta holds every key whose newest change the subscriber does not have yet, so a
 *  lost packet only makes a later delta carry that key again (deltaAgainstAck). Deltas go on lane 3 (or lane 0 for
 *  reliable grids and reliable keys) and are split to fit a datagram. A receiver applies a change only when it is
 *  newer than the one it has, applies received deltas to a back buffer and hands them to the application at the end
 *  of the session's tick, so the application always reads the state of one tick.
 *  Once the grid has its keys no allocation happens for numeric values.
 *
 *  Writes: the publisher writes directly. With HOST or PROPOSAL_VOTE authority a write on a replica becomes a
 *  proposal to the sequencer (ProposeResultEvent tells how it went); with OWNER authority it is refused.
 *  The methods are thread-safe through the lock of the node.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataGrid : public CDRNDataItem
{
    public:

    static const CORE::CEvent GridKeyChangedEvent;     /**< CORE::TCloneableUInt32: the key index; notify PER_CHANGE */
    static const CORE::CEvent GridUpdatedEvent;        /**< CDRNGridUpdatedEventData: the changed key indices; notify PER_PACKET */
    static const CORE::CEvent ProposeResultEvent;      /**< CDRNProposeResultEventData */
    static const CORE::CEvent VoteStartedEvent;        /**< CDRNVoteEventData: cast a vote with CastVote() */
    static const CORE::CEvent VoteEndedEvent;          /**< CDRNVoteEventData */

    static void RegisterEvents( void );

    static const UInt32 InvalidKeyIndex = 0xFFFFFFFF;

    /** The settings; on a replica those that travel in the announcement */
    const CDRNDataGridSettings& GetSettings( void ) const;

    /** How this replica (or origin) tells the application about changes */
    void SetNotifyMode( TDRNNotifyMode mode );

    /**
     *  Writes a value. On a replica of a HOST / PROPOSAL_VOTE grid this proposes the write and returns true when the
     *  proposal went out; on a replica of an OWNER grid it is refused.
     */
    bool SetValue( const CORE::CVariant& key, const CORE::CVariant& value );

    bool GetValue( const CORE::CVariant& key, CORE::CVariant& value ) const;

    bool HasKey( const CORE::CVariant& key ) const;

    bool RemoveKey( const CORE::CVariant& key );

    /** The dense index of a key, InvalidKeyIndex if the grid has no such key */
    UInt32 GetKeyIndex( const CORE::CVariant& key ) const;

    /** The index of a key; on the publisher the key is added when it is new (without a value) */
    UInt32 GetOrAddKeyIndex( const CORE::CVariant& key );

    /** The hot path: no lookup. Publisher only. */
    bool SetValueByIndex( UInt32 index, const CORE::CVariant& value );

    bool GetValueByIndex( UInt32 index, CORE::CVariant& value ) const;

    /** The key of an index, an empty variant for an unknown index */
    CORE::CVariant GetKeyByIndex( UInt32 index ) const;

    UInt32 GetKeyCount( void ) const;

    void GetKeys( GUCEF::vector< CORE::CVariant >& keys ) const;

    UInt32 GetRevision( UInt32 index ) const;

    /** The update tick (see GetUpdateTick) in which the application got the last change of the key */
    UInt32 GetLastChangeTick( UInt32 index ) const;

    /** The session time (microseconds) at which the last change of the key was sent */
    UInt64 GetLastChangeSendTime( UInt32 index ) const;

    /** The member that wrote the last change */
    UInt32 GetLastWriter( UInt32 index ) const;

    /** Counts the ticks in which the application got changes */
    UInt32 GetUpdateTick( void ) const;

    /** The keys whose change the application got after the given update tick (notify POLL) */
    void GetChangedSince( UInt32 updateTick, TDRNKeyIndexVector& indices ) const;

    /** Replica: asks the publisher for the whole state again */
    void RequestResync( void );

    /** Replica: votes on a write proposed to a PROPOSAL_VOTE grid */
    bool CastVote( UInt32 voteId, bool yes );

    /** Publisher: checks proposals before they are applied or put to a vote, NULL = the default (accept), not owned */
    void SetWriteValidator( CIDRNWriteValidator* validator );

    /** Bytes of deltas sent to subscribers */
    UInt64 GetBytesSent( void ) const;

    /** Delta messages sent to subscribers */
    UInt64 GetDeltasSent( void ) const;

    /** Deltas received */
    UInt64 GetDeltasReceived( void ) const;

    /** Times this replica asked for the whole state again */
    UInt32 GetResyncCount( void ) const;

    /** Every key, value, revision and writer; never used on the wire */
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Publisher only: writes the values of a dump */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    friend class CDRNReplicator;

    CDRNDataGrid( CDRNReplicator& replicator, const CDRNDataGridSettings& settings );
    virtual ~CDRNDataGrid();

    virtual void OnSubscriberAdded( UInt32 peerId, UInt32 flags, CDRNWireReader& params ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSubscriberRemoved( UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnItemMessage( UInt32 fromPeerId, UInt8 msgClass, UInt32 opcode, CDRNWireReader& reader ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnSendTick( UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnEndOfTick( UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void WriteAnnounceSettings( CDRNWireWriter& writer ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void ReadAnnounceSettings( CDRNWireReader& reader ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void WriteSubscribeParams( CDRNWireWriter& writer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnResubscribe( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnRemoved( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsDue( UInt64 nodeNow ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Float32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CKeySlot
    {
        public:
        CORE::CVariant key;
        CORE::CVariant value;           /**< the newest value: the publisher's, the back buffer of a replica */
        CORE::CVariant frontValue;      /**< what the application of a replica reads */
        CDRNQuantisation quantisation;
        UInt32 revision;
        UInt32 changeTick;              /**< write tick of the last change, what the deltas go by */
        UInt32 lastWriter;
        UInt64 changeSendTime;
        UInt32 rxSeq;                   /**< replica: the delta that brought the value */
        UInt32 frontRevision;
        UInt32 frontWriter;
        UInt64 frontSendTime;
        UInt32 updateTick;              /**< the update tick in which the application got the change */
        bool used;
        bool removed;
        bool frontRemoved;
        bool reliableKey;
        bool rxSeqValid;
        bool frontPending;

        CKeySlot( void );
    };

    class CKeyState
    {
        public:
        UInt32 sentChange;              /**< the change of the key last sent */
        UInt32 sentSeq;                 /**< the delta that carried it */
        UInt32 ackedChange;             /**< the newest change the subscriber is known to have */
        bool defined;                   /**< KEY_DEFINE was sent */

        CKeyState( void );
    };

    class CSubscriber
    {
        public:
        UInt32 peerId;
        bool needsSnapshot;
        UInt32 nextSeq;
        UInt32 lastReliableSeq;
        bool hasUnacked;
        UInt64 unackedSince;
        GUCEF::vector< CKeyState > keys;

        CSubscriber( void );
    };

    class CVote
    {
        public:
        UInt32 voteId;
        UInt32 proposerPeerId;
        UInt32 proposalId;
        CORE::CVariant key;
        CORE::CVariant value;
        bool isRemoval;
        UInt64 deadline;
        GUCEF::vector< UInt32 > voters;
        GUCEF::vector< UInt8 > votes;   /**< 0 = not yet, 1 = yes, 2 = no */

        CVote( void );
    };

    typedef GUCEF::vector< CKeySlot > TKeySlotVector;
    typedef GUCEF::map< CORE::CVariant, UInt32 > TKeyIndexMap;
    typedef GUCEF::vector< CSubscriber* > TSubscriberVector;
    typedef GUCEF::vector< CORE::CDynamicBuffer* > TBufferVector;
    typedef GUCEF::vector< CVote* > TVoteVector;

    CDRNDataGrid( const CDRNDataGrid& src );
    CDRNDataGrid& operator=( const CDRNDataGrid& src );

    UInt32 AddKey( const CORE::CVariant& key, UInt32 index, bool reliableKey, const CDRNQuantisation& quantisation );
    void EnsureSlotCount( UInt32 count );
    CSubscriber* FindSubscriber( UInt32 peerId ) const;
    void SendSnapshot( CSubscriber& subscriber, UInt64 nodeNow, UInt64 sessionTime );
    bool SendDeltas( CSubscriber& subscriber, UInt64 nodeNow, UInt64 sessionTime );
    void SendKeyDefines( CSubscriber& subscriber, const TDRNKeyIndexVector& indices );
    UInt32 SendDeltaMessages( CSubscriber& subscriber, const TDRNKeyIndexVector& indices, bool reliable, bool snapshot, UInt32 snapshotSeq, UInt64 nodeNow, UInt64 sessionTime, UInt32& budget );
    void SendBoundary( UInt32 peerId, UInt32 opcode, UInt32 seq );
    UInt64 GetResendTimeout( UInt32 peerId ) const;
    void OnAck( CSubscriber& subscriber, UInt32 ackSeq, UInt32 ackBits, UInt64 nodeNow );
    void OnDelta( CDRNWireReader& reader, bool replay );
    void OnKeyDefine( CDRNWireReader& reader );
    void OnSnapshotBegin( UInt32 seq );
    void OnSnapshotEnd( UInt32 seq );
    void OnPropose( UInt32 fromPeerId, CDRNWireReader& reader );
    void OnProposeResult( CDRNWireReader& reader );
    void OnVoteStart( CDRNWireReader& reader );
    void OnVoteCast( UInt32 fromPeerId, CDRNWireReader& reader );
    void OnVoteEnd( CDRNWireReader& reader );
    bool ApplyWrite( UInt32 index, const CORE::CVariant& value, bool remove, UInt32 writerPeerId );
    bool Propose( const CORE::CVariant& key, const CORE::CVariant& value, bool remove );
    void SendProposeResult( UInt32 proposerPeerId, UInt32 proposalId, bool accepted, UInt32 voteOutcome, const CORE::CString& reason );
    void StartVote( UInt32 proposerPeerId, UInt32 proposalId, const CORE::CVariant& key, const CORE::CVariant& value, bool remove );
    bool EvaluateVote( UInt32 voteSlot, bool timedOut, bool cancel );
    void StampChange( CKeySlot& slot );
    void MarkFrontPending( UInt32 index );
    void ClearBufferedDeltas( void );

    CDRNDataGridSettings m_settings;
    CIDRNWriteValidator* m_validator;
    CDRNDefaultWriteValidator m_defaultValidator;
    TKeySlotVector m_slots;
    TKeyIndexMap m_keyIndex;
    UInt32 m_keyCount;
    TSubscriberVector m_subscribers;

    // publishing
    UInt32 m_writeTick;
    bool m_writeTickUsed;
    UInt64 m_nextSendTime;
    bool m_sendPending;                 /**< something stayed behind or a subscriber waits for its snapshot */
    TDRNKeyIndexVector m_unreliableIndices;
    TDRNKeyIndexVector m_reliableIndices;
    TDRNKeyIndexVector m_defineIndices;
    UInt64 m_bytesSent;
    UInt64 m_deltasSent;
    CORE::CVariant m_txValue;

    // receiving
    CDRNSequenceWindow m_ackWindow;
    bool m_ackDue;
    UInt32 m_startSeq;
    UInt32 m_highestSeqSeen;
    bool m_anySeqSeen;
    UInt32 m_lastReliableSeq;
    bool m_inSnapshot;
    bool m_snapshotJustCompleted;
    UInt32 m_snapshotSeq;
    TBufferVector m_bufferedDeltas;
    UInt32 m_bufferedCount;
    UInt64 m_deltasReceived;
    UInt32 m_resyncCount;
    TDRNKeyIndexVector m_frontPending;
    UInt32 m_updateTick;
    CDRNGridUpdatedEventData m_updatedEventData;
    CORE::CVariant m_rxScratch;
    CORE::CVariant m_rxKey;

    // writes and votes
    UInt32 m_nextProposalId;
    UInt32 m_nextVoteId;
    TVoteVector m_votes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */
