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

#ifndef GUCEF_DRN_CDRNBULKTRANSFER_H
#define GUCEF_DRN_CDRNBULKTRANSFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#include "gucefDRN_CIDRNSessionComponent.h"
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

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

class CDRNSession;
class CDRNBulkTransferManager;

/** What the application learns about a transfer */
class GUCEF_DRN_EXPORT_CPP CDRNBulkTransferInfo
{
    public:

    UInt32 peerId;              /**< the other end */
    UInt32 transferId;          /**< unique per sending member */
    CORE::CString name;
    UInt64 size;
    bool isIncoming;

    CDRNBulkTransferInfo( void );
};

typedef CORE::CTCloneableObj< CDRNBulkTransferInfo > CDRNBulkTransferEventData;

/*-------------------------------------------------------------------------*/

/**
 *  One blob or file going to one member: OFFER {id, name, size, SHA-256}, ACCEPT {resume offset},
 *  CHUNK {offset, bytes} in 16 KB chunks on lane 1, DONE {hash ok} from the receiver once it checked the hash,
 *  CANCEL from either end. Hashing runs on a thread of the pool, so a large blob does not hold up the pulse.
 *  A receiver that accepts into a file it already has part of resumes where that file ends.
 *  Owned by the session's CDRNBulkTransferManager; events reach the application as the session's do.
 */
class GUCEF_DRN_EXPORT_CPP CDRNBulkTransfer : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent ProgressEvent;       /**< CORE::TCloneableUInt64: bytes done, at most once per tick */
    static const CORE::CEvent CompletedEvent;      /**< the receiver checked the hash */
    static const CORE::CEvent FailedEvent;         /**< CORE::TCloneableString: why */

    static void RegisterEvents( void );

    static const UInt32 ChunkSize = 16 * 1024;

    enum TTransferState
    {
        TRANSFERSTATE_HASHING           ,   /**< sender: hashing the source before the offer */
        TRANSFERSTATE_OFFERED           ,   /**< offered, the receiver did not answer yet */
        TRANSFERSTATE_TRANSFERRING      ,
        TRANSFERSTATE_VERIFYING         ,   /**< receiver: hashing what arrived; sender: waiting for DONE */
        TRANSFERSTATE_COMPLETED         ,
        TRANSFERSTATE_FAILED
    };

    /** Receiver: takes the offer into a file, resuming when the file has a part of it already */
    bool Accept( const CORE::CString& targetFilePath );

    /** Receiver: takes the offer into memory, see GetReceivedData() */
    bool Accept( void );

    /** Receiver: turns the offer down */
    void Reject( const CORE::CString& reason = CORE::CString::Empty );

    /** Either end: stops the transfer */
    void Cancel( const CORE::CString& reason = CORE::CString::Empty );

    TTransferState GetState( void ) const;
    bool IsFinished( void ) const;
    const CDRNBulkTransferInfo& GetInfo( void ) const;
    UInt64 GetBytesDone( void ) const;
    UInt64 GetResumeOffset( void ) const;
    const CORE::CString& GetFailureReason( void ) const;

    /** Receiver that accepted into memory: the data, once completed */
    const CORE::CDynamicBuffer& GetReceivedData( void ) const;

    const UInt8* GetSha256( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNBulkTransferManager;

    CDRNBulkTransfer( CDRNBulkTransferManager& manager );
    virtual ~CDRNBulkTransfer();
    CDRNBulkTransfer( const CDRNBulkTransfer& src );
    CDRNBulkTransfer& operator=( const CDRNBulkTransfer& src );

    bool StartHashing( void );
    bool TryTakeHash( UInt8* digest, UInt64& size, bool& ok );
    void Tick( UInt64 nodeNow );
    void OnMessage( UInt32 opcode, CDRNWireReader& reader );
    void SendChunks( void );
    void OnChunk( CDRNWireReader& reader );
    void Fail( const CORE::CString& reason, bool tellPeer );
    void Complete( void );
    bool ReadSource( UInt64 offset, UInt32 size, CORE::CDynamicBuffer& dest );
    void Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData = GUCEF_NULL );

    CDRNBulkTransferManager* m_manager;
    CDRNBulkTransferInfo m_info;
    TTransferState m_state;
    bool m_toFile;                          /**< the source (sender) or the target (receiver) is a file */
    CORE::CString m_filePath;
    CORE::CFileAccess m_file;
    CORE::CDynamicBuffer m_data;            /**< the source or target in memory */
    UInt8 m_sha256[ CORE::CSha256::DigestSize ];
    UInt64 m_resumeOffset;
    UInt64 m_nextOffset;                    /**< sender: the next chunk */
    UInt64 m_bytesDone;
    bool m_progressDue;
    CORE::CFutureResult m_hashFuture;
    CORE::CString m_failureReason;
    CORE::CDynamicBuffer m_chunk;
};

/*-------------------------------------------------------------------------*/

/**
 *  The bulk transfers of a session, the session component of the BULK message class. Transfers between members
 *  that have no direct link go through the sequencer, which passes the messages on.
 */
class GUCEF_DRN_EXPORT_CPP CDRNBulkTransferManager : public CORE::CTSGNotifier ,
                                                     public CIDRNSessionComponent
{
    public:

    static const CORE::CEvent IncomingTransferEvent;   /**< CDRNBulkTransferEventData: Accept() or Reject() the transfer, see GetIncoming() */

    static void RegisterEvents( void );

    explicit CDRNBulkTransferManager( CDRNSession& session );
    virtual ~CDRNBulkTransferManager();

    /** Offers a file to a member */
    CDRNBulkTransfer* OfferFile( UInt32 toPeerId, const CORE::CString& name, const CORE::CString& filePath );

    /** Offers a blob to a member, the data is copied */
    CDRNBulkTransfer* OfferData( UInt32 toPeerId, const CORE::CString& name, const CORE::CDynamicBuffer& data );

    /** An incoming transfer by sender and id, as told by IncomingTransferEvent */
    CDRNBulkTransfer* GetIncoming( UInt32 fromPeerId, UInt32 transferId ) const;

    /** Lets go of a finished transfer */
    bool Release( CDRNBulkTransfer* transfer );

    CDRNSession& GetSession( void ) const;

    /** A new run of the session: transfers end */
    void Reset( void );

    virtual UInt8 GetMessageClass( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionMessage( CDRNSession& session, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionTick( CDRNSession& session, UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnMemberLeft( CDRNSession& session, UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionClosed( CDRNSession& session ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNBulkTransfer;

    typedef GUCEF::vector< CDRNBulkTransfer* > TTransferVector;

    CDRNBulkTransferManager( const CDRNBulkTransferManager& src );
    CDRNBulkTransferManager& operator=( const CDRNBulkTransferManager& src );

    CDRNBulkTransfer* AddOutgoing( UInt32 toPeerId, const CORE::CString& name );
    CDRNBulkTransfer* FindTransfer( UInt32 peerId, UInt32 transferId, bool incoming ) const;
    bool Send( UInt32 toPeerId, UInt8 lane, UInt32 opcode, UInt32 senderPeerId, UInt32 transferId, const void* body, UInt32 bodySize );
    bool CanSendChunk( UInt32 toPeerId ) const;
    void Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData = GUCEF_NULL );

    CDRNSession* m_session;
    TTransferVector m_transfers;
    UInt32 m_nextTransferId;
    CORE::CDynamicBuffer m_txBuffer;
};

/*-------------------------------------------------------------------------*/

/** What a hash task works on and what came of it */
class GUCEF_DRN_EXPORT_CPP CDRNHashTaskData : public CORE::CICloneable
{
    public:

    bool fromFile;
    CORE::CString filePath;
    CORE::CDynamicBuffer data;
    UInt8 digest[ CORE::CSha256::DigestSize ];
    UInt64 size;
    bool succeeded;

    CDRNHashTaskData( void );
    CDRNHashTaskData( const CDRNHashTaskData& src );
    virtual ~CDRNHashTaskData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNHashTaskData& operator=( const CDRNHashTaskData& src );
};

/*-------------------------------------------------------------------------*/

/** Hashes a file or a blob with SHA-256 on a thread of the pool */
class GUCEF_DRN_EXPORT_CPP CDRNHashTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CDRNHashTaskConsumer( void );
    virtual ~CDRNHashTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNHashTaskConsumer( const CDRNHashTaskConsumer& src );
    CDRNHashTaskConsumer& operator=( const CDRNHashTaskConsumer& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNBULKTRANSFER_H ? */
