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

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFER_H
#define GUCEF_DRN_CDRNRESOURCETRANSFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_DRN_CDRNAPPLICATIONNOTIFIER_H
#include "gucefDRN_CDRNApplicationNotifier.h"
#define GUCEF_DRN_CDRNAPPLICATIONNOTIFIER_H
#endif /* GUCEF_DRN_CDRNAPPLICATIONNOTIFIER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCESINK_H
#include "gucefDRN_CIDRNResourceSink.h"
#define GUCEF_DRN_CIDRNRESOURCESINK_H
#endif /* GUCEF_DRN_CIDRNRESOURCESINK_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#include "gucefDRN_CIDRNResourceIoJob.h"
#define GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#endif /* GUCEF_DRN_CIDRNRESOURCEIOJOB_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#include "gucefDRN_CDRNResourceTransferOptions.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#include "gucefDRN_CDRNResourceTransferResult.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H ? */

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

class CDRNResourceSessionComponent;
class CDRNPeerLink;

/**
 *  One transfer of a resource, or of a window of it, from a member of the session into a sink: the requesting side.
 *  Made by CDRNResourceSessionComponent::Fetch() or Read(), let go of with its Release() once finished.
 *
 *  It opens the resource (OPEN, INFO), prepares the sink on the pool (which may resume), then reads the chunks that
 *  are missing with a credit it returns as the chunks are written, checks each against the hash tree when asked,
 *  writes them on the pool, runs the final checks there, and completes. Events reach the application as those of
 *  the session do (CDRNApplicationNotifier): OpenedEvent once, ProgressEvent at most once per tick, then
 *  CompletedEvent or FailedEvent once the sink was finished or aborted.
 *
 *  Calls take the node lock. WaitForCompletion() waits on the calling thread; it cannot be used inside work of the node
 *  (a handler the node delivers). The thread that pumps a node without a thread of its own (CDRNNode::IsPumpThread())
 *  works the node on that thread while it waits (CDRNNode::PumpUntil()), any other thread only waits.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceTransfer : public CDRNApplicationNotifier
{
    public:

    static const CORE::CEvent OpenedEvent;      /**< CDRNResourceInfoEventData: the resource as the serving side resolved it */
    static const CORE::CEvent ProgressEvent;    /**< CORE::TCloneableUInt64: bytes of the window done */
    static const CORE::CEvent CompletedEvent;   /**< everything was written, checked and finished in the sink */
    static const CORE::CEvent FailedEvent;      /**< CDRNResourceTransferResultEventData */

    static void RegisterEvents( void );

    enum TTransferState
    {
        TRANSFERSTATE_OPENING       ,   /**< OPEN sent, waiting for INFO */
        TRANSFERSTATE_PREPARING     ,   /**< the sink is made ready on the pool (resume) */
        TRANSFERSTATE_TRANSFERRING  ,
        TRANSFERSTATE_FINISHING     ,   /**< final checks and the sink's finish on the pool */
        TRANSFERSTATE_ABORTING      ,   /**< failed or cancelled, the sink is being aborted on the pool */
        TRANSFERSTATE_COMPLETED     ,
        TRANSFERSTATE_FAILED
    };

    TTransferState GetState( void ) const;
    bool IsFinished( void ) const;

    /** OK once completed; the status and words of the failure otherwise */
    CDRNResourceTransferResult GetResult( void ) const;

    /** The resource as the serving side resolved it, once opened */
    CDRNResourceInfo GetInfo( void ) const;

    UInt32 GetPeerId( void ) const;
    const CORE::CString& GetProviderName( void ) const;
    const CORE::CString& GetResourceName( void ) const;

    /** Bytes of the window written so far, those a resumed target held included */
    UInt64 GetBytesDone( void ) const;

    /** Bytes of the window, known once opened (the size of the resource for a fetch) */
    UInt64 GetBytesTotal( void ) const;

    /** Bytes that a resumed target held already */
    UInt64 GetBytesResumed( void ) const;

    /** Bytes of chunks that arrived over the link in this transfer */
    UInt64 GetBytesReceived( void ) const;

    const CIDRNResourceSinkPtr& GetSink( void ) const;

    /** Stops the transfer: CANCELLED; a resumable target keeps what it has, unless 'discard' */
    void Cancel( bool discard = false );

    /** Waits on this thread until the transfer finished or the time is up; false on time out or on a thread that works on the node */
    bool WaitForCompletion( UInt32 timeoutInMs );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNResourceSessionComponent;

    class CPipe;
    class CPrepareJob;
    class CDrainJob;
    class CFinishJob;
    class CAbortJob;

    typedef CORE::CTSharedPtr< CPipe, MT::CMutex > CPipePtr;
    typedef GUCEF::map< UInt64, UInt32 > TStrikeMap;

    CDRNResourceTransfer( CDRNResourceSessionComponent& component     ,
                          UInt32 peerId                               ,
                          UInt32 handle                               ,
                          const CORE::CString& providerName           ,
                          const CORE::CString& resourceName           ,
                          bool wholeResource                          ,
                          UInt64 windowOffset                         ,
                          UInt64 windowLength                         ,
                          const CIDRNResourceSinkPtr& sink            ,
                          const CDRNResourceTransferOptions& options  );
    virtual ~CDRNResourceTransfer();
    CDRNResourceTransfer( const CDRNResourceTransfer& src );               /**< not implemented */
    CDRNResourceTransfer& operator=( const CDRNResourceTransfer& src );    /**< not implemented */

    /** Node thread, under the node lock: sends OPEN */
    void Start( UInt64 nodeNowMs );
    void OnInfo( CDRNWireReader& reader );
    void OnData( UInt32 readId, UInt64 chunk, const UInt8* path, UInt32 pathLength, const UInt8* data, UInt32 dataSize );
    void OnReadEnd( UInt32 readId, TDRNResourceStatus status, const CORE::CString& message );
    void Tick( UInt64 nodeNowMs );

    /** Chunks of credit this transfer may still be given (0 when it waits for nothing) */
    UInt32 GetWantedCredit( void ) const;
    void AddPendingCredit( UInt32 chunks );
    void FlushCredit( void );

    void Fail( TDRNResourceStatus status, const CORE::CString& message, bool tellServer, bool discard = false, bool abortNow = false );
    void FinishWith( TTransferState finalState );
    void FinishAbortNow( void );
    void StartPrepare( bool resume );
    void StartNextRead( void );
    void CancelRead( void );
    void QueueDrain( void );
    void CollectDrain( void );
    void StartFinish( void );
    void SendClose( void );
    void Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData = GUCEF_NULL );
    bool IsActive( void ) const;
    UInt64 GetWindowChunkCount( void ) const;
    bool IsWindowComplete( void ) const;

    CDRNResourceSessionComponent* m_component;
    UInt32 m_peerId;
    UInt32 m_handle;
    CORE::CString m_providerName;
    CORE::CString m_resourceName;
    bool m_wholeResource;
    UInt64 m_windowOffset;
    UInt64 m_windowLength;
    CIDRNResourceSinkPtr m_sink;
    CDRNResourceTransferOptions m_options;
    TTransferState m_state;
    CDRNResourceTransferResult m_result;
    CDRNResourceInfo m_info;
    bool m_opened;                      /**< INFO said OK: the handle is open on the serving side */
    bool m_useTree;
    UInt64 m_firstChunk;                /**< the window in chunks */
    UInt64 m_endChunk;
    CDRNResourcePartialState m_seen;    /**< chunks held, received or written: not asked for again */
    UInt64 m_resumedChunks;
    UInt64 m_bytesResumed;
    UInt64 m_bytesDone;
    UInt64 m_bytesReceived;
    bool m_progressDue;
    bool m_restartedFresh;              /**< a final check failed over resumed data and the target started over once */
    TStrikeMap m_strikes;

    UInt32 m_readId;                    /**< the read under way, 0 for none */
    UInt32 m_nextReadId;
    UInt64 m_readStart;
    UInt64 m_readEnd;
    UInt64 m_readGranted;               /**< credit given for the read so far */
    UInt32 m_readOutstanding;           /**< credit given and not used by a chunk yet */
    bool m_readRequestPending;          /**< the READ goes out with the next credit */
    UInt64 m_readReceived;              /**< chunks of the read that arrived */
    UInt32 m_pendingCredit;
    UInt32 m_buffered;                  /**< chunks received and not yet written */
    UInt64 m_bufferedBytes;
    bool m_closeSent;

    CPipePtr m_pipe;
    CIDRNResourceIoJobPtr m_job;        /**< the prepare, finish or abort job */
    CORE::CFutureResult m_jobFuture;
    CIDRNResourceIoJobPtr m_drainJob;
    CORE::CFutureResult m_drainFuture;
    bool m_drainDue;

    UInt64 m_startedMs;
    UInt64 m_lastActivityMs;
    bool m_releaseWhenFinished;         /**< the component lets go of it once finished (a synchronous read that timed out) */
    MT::CSemaphore m_finished;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCETRANSFER_H ? */
