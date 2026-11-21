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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#include "gucefDRN_CDRNResourceHashTree.h"
#define GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHTREE_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#include "gucefDRN_CDRNResourceIoTaskConsumer.h"
#define GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#include "gucefDRN_CDRNResourceSessionComponent.h"
#define GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H ? */

#include "gucefDRN_CDRNResourceTransfer.h"

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

/** The largest window a transfer asks ahead for, in chunks */
static const UInt32 MaxWindowChunks = 1024;

/** Block size of the final checks reading a target back */
static const UInt32 ReadBackBlockSize = 1024 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the transfer shares with its jobs on the pool: the sink and its state (used under the sink lock, by one job at
 *  a time) and the chunks received and the results of the writes (under the queue lock, which the node's thread takes
 *  briefly). Shared by reference counting, so a transfer that went leaves its jobs harmless.
 */
class CDRNResourceTransfer::CPipe
{
    public:

    class CChunk
    {
        public:

        UInt64 index;
        CORE::CDynamicBuffer data;
        UInt32 pathLength;
        UInt8 path[ CDRNResourceHashTree::MaxAuditPathLength * CDRNResourceHashTree::HashSize ];

        CChunk( void ) : index( 0 ), data(), pathLength( 0 ) {}
    };

    class CDone
    {
        public:

        UInt64 index;
        UInt32 bytesWritten;
        UInt32 bytesReceived;
        bool verified;
        bool bad;

        CDone( void ) : index( 0 ), bytesWritten( 0 ), bytesReceived( 0 ), verified( false ), bad( false ) {}
    };

    typedef GUCEF::map< UInt64, CChunk* > TChunkMap;
    typedef GUCEF::vector< CDone > TDoneVector;

    MT::CMutex queueLock;
    MT::CMutex sinkLock;

    // set once before the first job, read only after
    CIDRNResourceSinkPtr sink;
    CORE::CString provider;
    CDRNResourceInfo info;
    UInt64 windowStart;
    UInt64 windowEnd;
    UInt64 firstChunk;
    bool useTree;
    bool inOrder;
    bool wholeResource;
    bool keepLeaves;
    bool checkSha256;
    UInt8 expectedSha256[ CDRNResourceInfo::DigestSize ];
    UInt32 saveIntervalMs;
    volatile bool cancelled;

    // under the sink lock
    CDRNResourcePartialState state;
    CDRNResourcePartialState verifiedRun;
    CDRNResourceHashTree leafTree;
    CDRNResourceHasher hasher;
    CDRNResourceHasher shaHasher;
    UInt64 nextInOrderChunk;
    UInt64 lastCommitMs;
    TDRNResourceStatus jobStatus;
    CORE::CString jobMessage;
    UInt64 resumedHeld;

    // under the queue lock
    TChunkMap pending;
    TDoneVector done;
    TDRNResourceStatus failure;
    CORE::CString failureMessage;

    CPipe( void )
        : queueLock()
        , sinkLock()
        , sink()
        , provider()
        , info()
        , windowStart( 0 )
        , windowEnd( 0 )
        , firstChunk( 0 )
        , useTree( false )
        , inOrder( false )
        , wholeResource( false )
        , keepLeaves( false )
        , checkSha256( false )
        , saveIntervalMs( 1000 )
        , cancelled( false )
        , state()
        , verifiedRun()
        , leafTree()
        , hasher()
        , shaHasher()
        , nextInOrderChunk( 0 )
        , lastCommitMs( 0 )
        , jobStatus( DRN_RESSTATUS_OK )
        , jobMessage()
        , resumedHeld( 0 )
        , pending()
        , done()
        , failure( DRN_RESSTATUS_OK )
        , failureMessage()
    {GUCEF_TRACE;

        memset( expectedSha256, 0, sizeof( expectedSha256 ) );
    }

    ~CPipe()
    {GUCEF_TRACE;

        for ( TChunkMap::iterator i=pending.begin(); i!=pending.end(); ++i )
            GUCEF_DELETE (*i).second;
        pending.clear();
    }

    /** Sink lock held: writes a chunk, trimmed to the window */
    TDRNResourceStatus WriteChunk( const CChunk& chunk, UInt32& bytesWritten )
    {GUCEF_TRACE;

        bytesWritten = 0;
        UInt64 start = chunk.index * DRN_RESOURCE_CHUNK_SIZE;
        UInt64 end = start + chunk.data.GetDataSize();
        UInt64 from = start > windowStart ? start : windowStart;
        UInt64 to = end < windowEnd ? end : windowEnd;
        if ( to <= from )
            return DRN_RESSTATUS_OK;
        const UInt8* bytes = static_cast< const UInt8* >( chunk.data.GetConstBufferPtr() ) + ( from - start );
        UInt32 size = (UInt32) ( to - from );
        TDRNResourceStatus status = sink->Write( from, bytes, size );
        if ( DRN_RESSTATUS_OK == status )
        {
            bytesWritten = size;
            if ( checkSha256 && inOrder )
                shaHasher.Update( bytes, size );
        }
        return status;
    }
};

/*-------------------------------------------------------------------------*/

/** Makes the sink ready: resume or start empty */
class CDRNResourceTransfer::CPrepareJob : public CIDRNResourceIoJob
{
    public:

    CPrepareJob( const CPipePtr& pipe, bool resume, bool restartIfChanged )
        : CIDRNResourceIoJob(), m_pipe( pipe ), m_resume( resume ), m_restartIfChanged( restartIfChanged )
    {GUCEF_TRACE;
    }

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CPipe& pipe = *m_pipe;
        MT::CScopeMutex lock( pipe.sinkLock );
        pipe.jobStatus = DRN_RESSTATUS_OK;
        if ( pipe.cancelled )
        {
            pipe.jobStatus = DRN_RESSTATUS_CANCELLED;
            return;
        }
        if ( m_resume && !m_restartIfChanged )
        {
            CDRNResourcePartialState persisted;
            if ( pipe.sink->PeekPartialState( persisted ) && persisted.GetProvider() == pipe.provider &&
                 persisted.GetName() == pipe.info.name && !persisted.IsOf( pipe.provider, pipe.info.name, pipe.info ) )
            {
                pipe.jobStatus = DRN_RESSTATUS_CHANGED;
                pipe.jobMessage = "the target holds part of another revision of the resource";
                return;
            }
        }
        pipe.jobStatus = pipe.sink->Begin( pipe.provider, pipe.info, pipe.windowStart, pipe.windowEnd - pipe.windowStart, m_resume, pipe.state );
        if ( DRN_RESSTATUS_OK != pipe.jobStatus )
        {
            pipe.jobMessage = "the target could not be prepared";
            return;
        }
        if ( !pipe.state.IsValid() && !pipe.state.Reset( pipe.provider, pipe.info.name, pipe.info ) )
        {
            pipe.jobStatus = DRN_RESSTATUS_TOO_LARGE;
            pipe.jobMessage = "the resource has too many chunks";
            return;
        }
        pipe.verifiedRun.Reset( pipe.provider, pipe.info.name, pipe.info );
        if ( pipe.keepLeaves && !pipe.leafTree.Reset( pipe.info.size ) )
            pipe.keepLeaves = false;
        pipe.nextInOrderChunk = pipe.firstChunk;
        if ( pipe.checkSha256 && pipe.inOrder )
            pipe.shaHasher.Begin();
        pipe.resumedHeld = pipe.state.GetHeldCount();
        pipe.lastCommitMs = CDRNClock::NowInMilliseconds();
    }

    private:

    CPipePtr m_pipe;
    bool m_resume;
    bool m_restartIfChanged;
};

/*-------------------------------------------------------------------------*/

/** Checks and writes the chunks that arrived */
class CDRNResourceTransfer::CDrainJob : public CIDRNResourceIoJob
{
    public:

    explicit CDrainJob( const CPipePtr& pipe )
        : CIDRNResourceIoJob(), m_pipe( pipe )
    {GUCEF_TRACE;
    }

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CPipe& pipe = *m_pipe;
        MT::CScopeMutex sinkLock( pipe.sinkLock );
        UInt64 leafCount = pipe.info.GetChunkCount();
        GUCEF::vector< CPipe::CChunk* > batch;
        while ( !pipe.cancelled )
        {
            batch.clear();
            {
                MT::CScopeMutex queueLock( pipe.queueLock );
                if ( DRN_RESSTATUS_OK != pipe.failure )
                    break;
                if ( pipe.inOrder )
                {
                    CPipe::TChunkMap::iterator i = pipe.pending.find( pipe.nextInOrderChunk );
                    UInt64 next = pipe.nextInOrderChunk;
                    while ( i != pipe.pending.end() && (*i).first == next )
                    {
                        batch.push_back( (*i).second );
                        pipe.pending.erase( i++ );
                        ++next;
                    }
                }
                else
                {
                    for ( CPipe::TChunkMap::iterator i=pipe.pending.begin(); i!=pipe.pending.end(); ++i )
                        batch.push_back( (*i).second );
                    pipe.pending.clear();
                }
            }
            if ( batch.empty() )
                break;

            CPipe::TDoneVector results;
            TDRNResourceStatus failure = DRN_RESSTATUS_OK;
            for ( UInt32 n=0; n<batch.size(); ++n )
            {
                CPipe::CChunk* chunk = batch[ n ];
                CPipe::CDone result;
                result.index = chunk->index;
                result.bytesReceived = chunk->data.GetDataSize();
                if ( DRN_RESSTATUS_OK == failure && !pipe.cancelled )
                {
                    bool good = true;
                    if ( pipe.useTree )
                    {
                        UInt8 leaf[ CDRNResourceHashTree::HashSize ];
                        pipe.hasher.HashLeaf( chunk->data.GetConstBufferPtr(), chunk->data.GetDataSize(), leaf );
                        good = CDRNResourceHashTree::VerifyAuditPath( pipe.hasher, chunk->index, leafCount, leaf, chunk->path, chunk->pathLength, pipe.info.treeRoot );
                        result.verified = good;
                        if ( good && pipe.keepLeaves )
                            pipe.leafTree.SetLeafHash( chunk->index, leaf );
                    }
                    if ( !good )
                    {
                        result.bad = true;
                    }
                    else
                    {
                        UInt32 written = 0;
                        failure = pipe.WriteChunk( *chunk, written );
                        if ( DRN_RESSTATUS_OK == failure )
                        {
                            result.bytesWritten = written;
                            pipe.state.SetHeld( chunk->index, true );
                            pipe.verifiedRun.SetHeld( chunk->index, result.verified );
                            if ( pipe.inOrder )
                                pipe.nextInOrderChunk = chunk->index + 1;
                        }
                    }
                }
                results.push_back( result );
                GUCEF_DELETE chunk;
            }

            UInt64 now = CDRNClock::NowInMilliseconds();
            if ( DRN_RESSTATUS_OK == failure && pipe.sink->IsResumable() && now >= pipe.lastCommitMs + pipe.saveIntervalMs )
            {
                failure = pipe.sink->Commit( pipe.state );
                pipe.lastCommitMs = now;
            }

            MT::CScopeMutex queueLock( pipe.queueLock );
            pipe.done.insert( pipe.done.end(), results.begin(), results.end() );
            if ( DRN_RESSTATUS_OK != failure && DRN_RESSTATUS_OK == pipe.failure )
            {
                pipe.failure = failure;
                pipe.failureMessage = "the target could not be written";
            }
        }
    }

    private:

    CPipePtr m_pipe;
};

/*-------------------------------------------------------------------------*/

/** The final checks, then the sink's finish */
class CDRNResourceTransfer::CFinishJob : public CIDRNResourceIoJob
{
    public:

    explicit CFinishJob( const CPipePtr& pipe )
        : CIDRNResourceIoJob(), m_pipe( pipe )
    {GUCEF_TRACE;
    }

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CPipe& pipe = *m_pipe;
        MT::CScopeMutex lock( pipe.sinkLock );
        pipe.jobStatus = DRN_RESSTATUS_OK;
        pipe.jobMessage.Clear();
        if ( pipe.cancelled )
        {
            pipe.jobStatus = DRN_RESSTATUS_CANCELLED;
            return;
        }

        CORE::CDynamicBuffer block;

        // chunks kept from an earlier transfer were not checked in this one: the tree over every chunk has to come out
        // at the root of the resource
        if ( pipe.useTree && pipe.keepLeaves && pipe.resumedHeld > 0 )
        {
            for ( UInt64 i=pipe.state.FindFirstHeld( 0 ); i<pipe.state.GetChunkCount(); i=pipe.state.FindFirstHeld( i+1 ) )
            {
                if ( pipe.verifiedRun.IsHeld( i ) )
                    continue;
                UInt32 size = pipe.info.GetChunkBytes( i );
                block.SetDataSize( size );
                if ( DRN_RESSTATUS_OK != pipe.sink->ReadBack( i * DRN_RESOURCE_CHUNK_SIZE, size, block.GetBufferPtr() ) )
                {
                    Refuse( DRN_RESSTATUS_LOCAL_IO_ERROR, "the target cannot be read back for its final check" );
                    return;
                }
                UInt8 leaf[ CDRNResourceHashTree::HashSize ];
                pipe.hasher.HashLeaf( block.GetConstBufferPtr(), size, leaf );
                pipe.leafTree.SetLeafHash( i, leaf );
            }
            UInt8 root[ CDRNResourceHashTree::HashSize ];
            if ( !pipe.leafTree.Finish( pipe.hasher ) || !pipe.leafTree.GetRoot( root ) ||
                 !CORE::CSha256::ConstantTimeEquals( root, pipe.info.treeRoot, CDRNResourceHashTree::HashSize ) )
            {
                Refuse( DRN_RESSTATUS_DIGEST_MISMATCH, "the target does not match the hash tree of the resource" );
                return;
            }
        }

        if ( pipe.checkSha256 )
        {
            UInt8 digest[ CDRNResourceInfo::DigestSize ];
            if ( pipe.inOrder )
            {
                pipe.shaHasher.Final( digest );
            }
            else
            {
                pipe.hasher.Begin();
                for ( UInt64 offset=0; offset<pipe.info.size; )
                {
                    UInt64 left = pipe.info.size - offset;
                    UInt32 size = left < ReadBackBlockSize ? (UInt32) left : ReadBackBlockSize;
                    block.SetDataSize( size );
                    TDRNResourceStatus status = pipe.sink->ReadBack( offset, size, block.GetBufferPtr() );
                    if ( DRN_RESSTATUS_OK != status )
                    {
                        Refuse( DRN_RESSTATUS_UNSUPPORTED == status ? status : DRN_RESSTATUS_LOCAL_IO_ERROR, "the target cannot be read back for its SHA-256 check" );
                        return;
                    }
                    pipe.hasher.Update( block.GetConstBufferPtr(), size );
                    offset += size;
                }
                pipe.hasher.Final( digest );
            }
            if ( !CORE::CSha256::ConstantTimeEquals( digest, pipe.expectedSha256, sizeof( digest ) ) )
            {
                Refuse( DRN_RESSTATUS_DIGEST_MISMATCH, "the resource does not have the SHA-256 that was expected" );
                return;
            }
        }

        pipe.jobStatus = pipe.sink->Finish();
        if ( DRN_RESSTATUS_OK != pipe.jobStatus )
            pipe.jobMessage = "the target could not be finished";
    }

    private:

    void Refuse( TDRNResourceStatus status, const char* message )
    {GUCEF_TRACE;

        CPipe& pipe = *m_pipe;
        pipe.jobStatus = status;
        pipe.jobMessage = message;
        pipe.sink->Abort( pipe.state, true );
    }

    CPipePtr m_pipe;
};

/*-------------------------------------------------------------------------*/

/** The sink's abort: a resumable target keeps what it holds */
class CDRNResourceTransfer::CAbortJob : public CIDRNResourceIoJob
{
    public:

    CAbortJob( const CPipePtr& pipe, bool discard )
        : CIDRNResourceIoJob(), m_pipe( pipe ), m_discard( discard )
    {GUCEF_TRACE;
    }

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CPipe& pipe = *m_pipe;
        MT::CScopeMutex lock( pipe.sinkLock );
        if ( pipe.state.IsValid() )
            pipe.sink->Abort( pipe.state, m_discard );
    }

    private:

    CPipePtr m_pipe;
    bool m_discard;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNResourceTransfer::OpenedEvent = "GUCEF::DRN::CDRNResourceTransfer::OpenedEvent";
const CORE::CEvent CDRNResourceTransfer::ProgressEvent = "GUCEF::DRN::CDRNResourceTransfer::ProgressEvent";
const CORE::CEvent CDRNResourceTransfer::CompletedEvent = "GUCEF::DRN::CDRNResourceTransfer::CompletedEvent";
const CORE::CEvent CDRNResourceTransfer::FailedEvent = "GUCEF::DRN::CDRNResourceTransfer::FailedEvent";

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::RegisterEvents( void )
{GUCEF_TRACE;

    OpenedEvent.Initialize();
    ProgressEvent.Initialize();
    CompletedEvent.Initialize();
    FailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransfer::CDRNResourceTransfer( CDRNResourceSessionComponent& component     ,
                                            UInt32 peerId                               ,
                                            UInt32 handle                               ,
                                            const CORE::CString& providerName           ,
                                            const CORE::CString& resourceName           ,
                                            bool wholeResource                          ,
                                            UInt64 windowOffset                         ,
                                            UInt64 windowLength                         ,
                                            const CIDRNResourceSinkPtr& sink            ,
                                            const CDRNResourceTransferOptions& options  )
    : CDRNApplicationNotifier( component.GetSession().GetNode() )
    , m_component( &component )
    , m_peerId( peerId )
    , m_handle( handle )
    , m_providerName( providerName )
    , m_resourceName( resourceName )
    , m_wholeResource( wholeResource )
    , m_windowOffset( windowOffset )
    , m_windowLength( windowLength )
    , m_sink( sink )
    , m_options( options )
    , m_state( TRANSFERSTATE_OPENING )
    , m_result()
    , m_info()
    , m_opened( false )
    , m_useTree( false )
    , m_firstChunk( 0 )
    , m_endChunk( 0 )
    , m_seen()
    , m_resumedChunks( 0 )
    , m_bytesResumed( 0 )
    , m_bytesDone( 0 )
    , m_bytesReceived( 0 )
    , m_progressDue( false )
    , m_restartedFresh( false )
    , m_strikes()
    , m_readId( 0 )
    , m_nextReadId( 1 )
    , m_readStart( 0 )
    , m_readEnd( 0 )
    , m_readGranted( 0 )
    , m_readOutstanding( 0 )
    , m_readRequestPending( false )
    , m_readReceived( 0 )
    , m_pendingCredit( 0 )
    , m_buffered( 0 )
    , m_bufferedBytes( 0 )
    , m_closeSent( false )
    , m_pipe()
    , m_job()
    , m_jobFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_drainJob()
    , m_drainFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_drainDue( false )
    , m_startedMs( 0 )
    , m_lastActivityMs( 0 )
    , m_releaseWhenFinished( false )
    , m_finished( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    if ( 0 == m_options.windowChunks )
        m_options.windowChunks = 1;
    if ( m_options.windowChunks > MaxWindowChunks )
        m_options.windowChunks = MaxWindowChunks;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransfer::~CDRNResourceTransfer()
{GUCEF_TRACE;

    if ( !m_pipe.IsNULL() )
        m_pipe->cancelled = true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourceTransfer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNResourceTransfer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    EmitToApplication( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransfer::TTransferState
CDRNResourceTransfer::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransfer::IsFinished( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return TRANSFERSTATE_COMPLETED == m_state || TRANSFERSTATE_FAILED == m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransfer::IsActive( void ) const
{GUCEF_TRACE;

    return TRANSFERSTATE_COMPLETED != m_state && TRANSFERSTATE_FAILED != m_state && TRANSFERSTATE_ABORTING != m_state;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceTransfer::GetResult( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_result;
}

/*-------------------------------------------------------------------------*/

CDRNResourceInfo
CDRNResourceTransfer::GetInfo( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_info;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceTransfer::GetPeerId( void ) const
{GUCEF_TRACE;

    return m_peerId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourceTransfer::GetProviderName( void ) const
{GUCEF_TRACE;

    return m_providerName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourceTransfer::GetResourceName( void ) const
{GUCEF_TRACE;

    return m_resourceName;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceTransfer::GetBytesDone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_bytesDone;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceTransfer::GetBytesTotal( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_windowLength;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceTransfer::GetBytesResumed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_bytesResumed;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceTransfer::GetBytesReceived( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    return m_bytesReceived;
}

/*-------------------------------------------------------------------------*/

const CIDRNResourceSinkPtr&
CDRNResourceTransfer::GetSink( void ) const
{GUCEF_TRACE;

    return m_sink;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceTransfer::GetWindowChunkCount( void ) const
{GUCEF_TRACE;

    return m_endChunk - m_firstChunk;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransfer::IsWindowComplete( void ) const
{GUCEF_TRACE;

    return m_seen.FindFirstMissing( m_firstChunk ) >= m_endChunk;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::Cancel( bool discard )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_component->GetSession().GetNode().GetLock() );
    if ( !IsActive() )
        return;
    Fail( DRN_RESSTATUS_CANCELLED, "cancelled", true, discard );
}

/*-------------------------------------------------------------------------*/

/** What a thread that pumps the node waits for in WaitForCompletion(): the transfer to be finished */
class CTransferFinishedCondition : public CIDRNWaitCondition
{
    public:

    explicit CTransferFinishedCondition( const CDRNResourceTransfer& transfer ) : m_transfer( transfer ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_transfer.IsFinished();
    }

    private:

    CTransferFinishedCondition& operator=( const CTransferFinishedCondition& src );
    const CDRNResourceTransfer& m_transfer;
};

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransfer::WaitForCompletion( UInt32 timeoutInMs )
{GUCEF_TRACE;

    CDRNNode& node = m_component->GetSession().GetNode();
    {
        MT::CScopeMutex lock( node.GetLock() );
        if ( TRANSFERSTATE_COMPLETED == m_state || TRANSFERSTATE_FAILED == m_state )
            return true;
        if ( node.IsWorkingOnThisThread() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceTransfer: Waiting for \"" << m_resourceName << "\" on a thread that works on the node would never end" );
            return false;
        }
    }

    // the thread that pumps the node works the node itself while it waits: the transfer finishes in the cycles of the node
    if ( node.IsPumpThread() )
    {
        CTransferFinishedCondition condition( *this );
        return node.PumpUntil( condition, timeoutInMs );
    }

    if ( 0 != m_finished.WaitOrTimeout( timeoutInMs ) )
        return false;
    m_finished.Post();      // the next wait passes as well
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::Start( UInt64 nodeNowMs )
{GUCEF_TRACE;

    m_startedMs = nodeNowMs;
    m_lastActivityMs = nodeNowMs;

    TDRNResourceStatus status = DRN_RESSTATUS_OK;
    CORE::CString why;
    CDRNPeerLinkPtr link = m_component->GetUsableLink( m_peerId, status, why );
    if ( link.IsNULL() )
    {
        Fail( status, why, false );
        return;
    }
    if ( m_sink.IsNULL() )
    {
        Fail( DRN_RESSTATUS_LOCAL_IO_ERROR, "no target", false );
        return;
    }

    m_component->m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_component->m_txBuffer );
    writer.WriteVarUInt32( m_handle );
    writer.WriteString( m_providerName );
    writer.WriteString( m_resourceName );
    writer.WriteVarUInt32( m_options.WantsTree() ? DRN_RES_OPEN_FLAG_TREE : 0 );
    writer.WriteString( m_options.expectedRevision );
    if ( !m_component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_OPEN, m_component->m_txBuffer.GetConstBufferPtr(), writer.GetOffset() ) )
    {
        Fail( DRN_RESSTATUS_LINK_LOST, "the open could not be sent", false );
        return;
    }
    ++m_component->m_stats.transfersStarted;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNResourceTransfer: Opening \"" << m_providerName << "/" << m_resourceName << "\" of member " << m_peerId );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::OnInfo( CDRNWireReader& reader )
{GUCEF_TRACE;

    if ( TRANSFERSTATE_OPENING != m_state )
        return;

    UInt32 statusValue = 0;
    CORE::CString message;
    if ( !reader.ReadVarUInt32( statusValue ) || !reader.ReadString( message ) )
    {
        Fail( DRN_RESSTATUS_PROTOCOL_ERROR, "a malformed INFO", true );
        return;
    }
    TDRNResourceStatus status = CDRNResourceTransferResult::StatusFromWire( statusValue );
    if ( DRN_RESSTATUS_OK != status )
    {
        // the serving side keeps no handle for a refused open
        m_closeSent = true;
        Fail( status, message, false );
        return;
    }

    CDRNResourceInfo info;
    UInt32 chunkSize = 0;
    UInt8 flags = 0;
    if ( !reader.ReadVarUInt64( info.size ) || !reader.ReadVarUInt64( info.modifiedUtcMs ) || !reader.ReadString( info.revision ) ||
         !reader.ReadVarUInt32( chunkSize ) || !reader.ReadUInt8( flags ) )
    {
        Fail( DRN_RESSTATUS_PROTOCOL_ERROR, "a malformed INFO", true );
        return;
    }
    info.name = m_resourceName;
    info.hasSha256 = 0 != ( flags & DRN_RES_DIGEST_SHA256 );
    info.hasTreeRoot = 0 != ( flags & DRN_RES_DIGEST_TREE );
    if ( ( info.hasSha256 && !reader.ReadBytes( info.sha256, CDRNResourceInfo::DigestSize ) ) ||
         ( info.hasTreeRoot && !reader.ReadBytes( info.treeRoot, CDRNResourceInfo::DigestSize ) ) )
    {
        Fail( DRN_RESSTATUS_PROTOCOL_ERROR, "a malformed INFO", true );
        return;
    }
    m_opened = true;
    m_info = info;
    m_lastActivityMs = CDRNClock::NowInMilliseconds();

    if ( DRN_RESOURCE_CHUNK_SIZE != chunkSize )
    {
        Fail( DRN_RESSTATUS_PROTOCOL_ERROR, "another chunk size than this node knows", true );
        return;
    }
    if ( m_options.RequiresTree() && !info.hasTreeRoot )
    {
        Fail( DRN_RESSTATUS_NO_TREE, "the serving side has no hash tree of the resource", true );
        return;
    }
    if ( m_options.hasTreeDigest )
    {
        UInt8 digest[ CDRNResourceInfo::DigestSize ];
        CDRNResourceHasher hasher;
        CDRNResourceHashTree::ComputeDigest( hasher, info.size, info.treeRoot, digest );
        if ( !CORE::CSha256::ConstantTimeEquals( digest, m_options.treeDigest, sizeof( digest ) ) )
        {
            Fail( DRN_RESSTATUS_DIGEST_MISMATCH, "the resource does not have the digest that was expected", true );
            return;
        }
    }
    if ( m_options.hasSha256 && info.hasSha256 && !CORE::CSha256::ConstantTimeEquals( info.sha256, m_options.sha256, CDRNResourceInfo::DigestSize ) )
    {
        Fail( DRN_RESSTATUS_DIGEST_MISMATCH, "the resource does not have the SHA-256 that was expected", true );
        return;
    }
    m_useTree = info.hasTreeRoot && m_options.WantsTree();

    if ( m_wholeResource )
    {
        m_windowOffset = 0;
        m_windowLength = info.size;
    }
    else if ( m_windowOffset > info.size || m_windowLength > info.size - m_windowOffset )
    {
        Fail( DRN_RESSTATUS_OUT_OF_RANGE, "the window reaches beyond the end of the resource", true );
        return;
    }
    m_firstChunk = m_windowOffset / DRN_RESOURCE_CHUNK_SIZE;
    m_endChunk = 0 == m_windowLength ? m_firstChunk : ( m_windowOffset + m_windowLength + DRN_RESOURCE_CHUNK_SIZE - 1 ) / DRN_RESOURCE_CHUNK_SIZE;

    CDRNResourceInfoEventData eventData( m_info );
    Emit( OpenedEvent, &eventData );
    if ( TRANSFERSTATE_OPENING == m_state )
        StartPrepare( m_options.resume && m_wholeResource );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::StartPrepare( bool resume )
{GUCEF_TRACE;

    m_state = TRANSFERSTATE_PREPARING;
    m_pipe = CPipePtr( GUCEF_NEW CPipe() );
    CPipe& pipe = *m_pipe;
    pipe.sink = m_sink;
    pipe.provider = m_providerName;
    pipe.info = m_info;
    pipe.windowStart = m_windowOffset;
    pipe.windowEnd = m_windowOffset + m_windowLength;
    pipe.firstChunk = m_firstChunk;
    pipe.useTree = m_useTree;
    pipe.inOrder = m_sink->IsInOrder();
    pipe.wholeResource = m_wholeResource;
    pipe.keepLeaves = m_useTree && m_wholeResource && m_sink->IsResumable();
    pipe.checkSha256 = m_wholeResource && ( m_options.hasSha256 || m_info.hasSha256 );
    if ( m_options.hasSha256 )
        memcpy( pipe.expectedSha256, m_options.sha256, sizeof( pipe.expectedSha256 ) );
    else
        memcpy( pipe.expectedSha256, m_info.sha256, sizeof( pipe.expectedSha256 ) );
    pipe.saveIntervalMs = m_component->m_settings.stateSaveIntervalMs;

    m_job = CIDRNResourceIoJobPtr( GUCEF_NEW CPrepareJob( m_pipe, resume, m_options.restartIfChanged ) );
    m_jobFuture = CDRNResourceIoTaskConsumer::Queue( m_component->m_settings.threadPoolName, m_job, m_component->GetSession().GetNode().GetWorkPulseGenerator() );
    if ( m_jobFuture.HasNoFuture() )
        Fail( DRN_RESSTATUS_LOCAL_IO_ERROR, "no thread pool to prepare the target on", true );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::StartNextRead( void )
{GUCEF_TRACE;

    UInt64 first = m_seen.FindFirstMissing( m_firstChunk );
    if ( first >= m_endChunk )
        return;
    UInt64 end = m_seen.FindFirstHeld( first );
    if ( end > m_endChunk )
        end = m_endChunk;
    m_readId = m_nextReadId++;
    if ( 0 == m_nextReadId )
        m_nextReadId = 1;
    m_readStart = first;
    m_readEnd = end;
    m_readGranted = 0;
    m_readOutstanding = 0;
    m_readReceived = 0;
    m_readRequestPending = true;
    m_pendingCredit = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::CancelRead( void )
{GUCEF_TRACE;

    if ( 0 == m_readId )
        return;
    if ( !m_readRequestPending )
    {
        UInt8 body[ 16 ];
        UInt32 size = CDRNWireWriter::EncodeVarUInt64( m_handle, body );
        size += CDRNWireWriter::EncodeVarUInt64( m_readId, body + size );
        m_component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_CANCEL, body, size );
    }
    m_component->AddLinkOutstanding( m_peerId, -(Int64) m_readOutstanding );
    m_readOutstanding = 0;
    m_pendingCredit = 0;
    m_readRequestPending = false;
    m_readId = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceTransfer::GetWantedCredit( void ) const
{GUCEF_TRACE;

    if ( TRANSFERSTATE_TRANSFERRING != m_state || 0 == m_readId )
        return 0;
    UInt64 used = (UInt64) m_readOutstanding + m_buffered + m_pendingCredit;
    if ( used >= m_options.windowChunks )
        return 0;
    UInt64 unasked = ( m_readEnd - m_readStart ) - m_readGranted - m_pendingCredit;
    UInt64 room = m_options.windowChunks - used;
    return (UInt32) ( room < unasked ? room : unasked );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::AddPendingCredit( UInt32 chunks )
{GUCEF_TRACE;

    m_pendingCredit += chunks;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::FlushCredit( void )
{GUCEF_TRACE;

    if ( 0 == m_pendingCredit || 0 == m_readId )
        return;
    UInt32 credit = m_pendingCredit;
    m_pendingCredit = 0;
    m_component->m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_component->m_txBuffer );
    writer.WriteVarUInt32( m_handle );
    writer.WriteVarUInt32( m_readId );
    UInt32 opcode = DRN_RES_CREDIT;
    if ( m_readRequestPending )
    {
        writer.WriteVarUInt64( m_readStart );
        writer.WriteVarUInt64( m_readEnd - m_readStart );
        opcode = DRN_RES_READ;
    }
    writer.WriteVarUInt32( credit );
    if ( !m_component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, opcode, m_component->m_txBuffer.GetConstBufferPtr(), writer.GetOffset() ) )
    {
        Fail( DRN_RESSTATUS_LINK_LOST, "the link went down", false );
        return;
    }
    if ( m_readRequestPending )
    {
        m_readRequestPending = false;
        m_lastActivityMs = CDRNClock::NowInMilliseconds();
    }
    m_readGranted += credit;
    m_readOutstanding += credit;
    m_component->AddLinkOutstanding( m_peerId, (Int64) credit );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::OnData( UInt32 readId, UInt64 chunk, const UInt8* path, UInt32 pathLength, const UInt8* data, UInt32 dataSize )
{GUCEF_TRACE;

    if ( TRANSFERSTATE_TRANSFERRING != m_state || 0 == m_readId || readId != m_readId ||
         chunk < m_readStart || chunk >= m_readEnd || m_seen.IsHeld( chunk ) || 0 == m_readOutstanding )
    {
        ++m_component->m_stats.droppedMessages;
        return;
    }
    if ( dataSize != m_info.GetChunkBytes( chunk ) || ( m_useTree && 0 == pathLength && m_info.GetChunkCount() > 1 ) )
    {
        Fail( DRN_RESSTATUS_PROTOCOL_ERROR, "a chunk of the wrong size or without its audit path", true );
        return;
    }

    CPipe::CChunk* entry = GUCEF_NEW CPipe::CChunk();
    entry->index = chunk;
    entry->data.CopyFrom( dataSize, data );
    entry->pathLength = pathLength;
    if ( pathLength > 0 )
        memcpy( entry->path, path, pathLength * CDRNResourceHashTree::HashSize );
    {
        MT::CScopeMutex queueLock( m_pipe->queueLock );
        CPipe::TChunkMap::iterator i = m_pipe->pending.find( chunk );
        if ( i != m_pipe->pending.end() )
        {
            GUCEF_DELETE (*i).second;
            (*i).second = entry;
        }
        else
        {
            m_pipe->pending[ chunk ] = entry;
        }
    }

    m_seen.SetHeld( chunk, true );
    --m_readOutstanding;
    m_component->AddLinkOutstanding( m_peerId, -1 );
    ++m_readReceived;
    ++m_buffered;
    m_bufferedBytes += dataSize;
    m_component->UpdateClientBuffered( (Int64) dataSize );
    m_bytesReceived += dataSize;
    ++m_component->m_stats.chunksReceived;
    m_component->m_stats.bytesReceived += dataSize;
    m_lastActivityMs = CDRNClock::NowInMilliseconds();

    if ( m_readReceived >= m_readEnd - m_readStart )
    {
        // the read is done: the serving side ends it by itself
        m_component->AddLinkOutstanding( m_peerId, -(Int64) m_readOutstanding );
        m_readOutstanding = 0;
        m_readId = 0;
    }

    m_drainDue = true;
    if ( m_drainFuture.HasNoFuture() )
        QueueDrain();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::OnReadEnd( UInt32 readId, TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    if ( readId != m_readId || !IsActive() )
        return;
    Fail( DRN_RESSTATUS_OK == status ? DRN_RESSTATUS_PROTOCOL_ERROR : status, message, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::QueueDrain( void )
{GUCEF_TRACE;

    m_drainDue = false;
    m_drainJob = CIDRNResourceIoJobPtr( GUCEF_NEW CDrainJob( m_pipe ) );
    m_drainFuture = CDRNResourceIoTaskConsumer::Queue( m_component->m_settings.threadPoolName, m_drainJob, m_component->GetSession().GetNode().GetWorkPulseGenerator() );
    if ( m_drainFuture.HasNoFuture() )
    {
        m_drainJob.Unlink();
        Fail( DRN_RESSTATUS_LOCAL_IO_ERROR, "no thread pool to write the target on", true );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::CollectDrain( void )
{GUCEF_TRACE;

    m_drainFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_drainJob.Unlink();

    CPipe::TDoneVector done;
    TDRNResourceStatus failure = DRN_RESSTATUS_OK;
    CORE::CString failureMessage;
    {
        MT::CScopeMutex queueLock( m_pipe->queueLock );
        done.swap( m_pipe->done );
        failure = m_pipe->failure;
        failureMessage = m_pipe->failureMessage;
    }

    bool restartRead = false;
    for ( UInt32 i=0; i<done.size(); ++i )
    {
        const CPipe::CDone& result = done[ i ];
        if ( m_buffered > 0 )
            --m_buffered;
        UInt64 bytes = result.bytesReceived < m_bufferedBytes ? result.bytesReceived : m_bufferedBytes;
        m_bufferedBytes -= bytes;
        m_component->UpdateClientBuffered( -(Int64) bytes );
        if ( result.verified )
            ++m_component->m_stats.chunksVerified;
        if ( result.bad )
        {
            ++m_component->m_stats.verificationFailures;
            UInt32 strikes = ++m_strikes[ result.index ];
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceTransfer: Chunk " << result.index << " of \"" << m_providerName << "/" << m_resourceName <<
                               "\" from member " << m_peerId << " failed its check (" << strikes << ")" );
            if ( strikes > m_options.corruptChunkRetries )
            {
                Fail( DRN_RESSTATUS_DIGEST_MISMATCH, "a chunk kept failing its check against the hash tree", true );
                return;
            }
            m_seen.SetHeld( result.index, false );
            restartRead = true;
            continue;
        }
        m_bytesDone += result.bytesWritten;
        m_progressDue = true;
    }
    if ( DRN_RESSTATUS_OK != failure )
    {
        Fail( failure, failureMessage, true );
        return;
    }
    if ( restartRead )
    {
        // the chunk is asked for again with the rest of what is missing; what arrives twice is dropped
        CancelRead();
    }
    if ( m_drainDue )
        QueueDrain();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::StartFinish( void )
{GUCEF_TRACE;

    m_state = TRANSFERSTATE_FINISHING;
    m_job = CIDRNResourceIoJobPtr( GUCEF_NEW CFinishJob( m_pipe ) );
    m_jobFuture = CDRNResourceIoTaskConsumer::Queue( m_component->m_settings.threadPoolName, m_job, m_component->GetSession().GetNode().GetWorkPulseGenerator() );
    if ( m_jobFuture.HasNoFuture() )
        Fail( DRN_RESSTATUS_LOCAL_IO_ERROR, "no thread pool to finish the target on", true );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::SendClose( void )
{GUCEF_TRACE;

    if ( m_closeSent )
        return;
    m_closeSent = true;
    UInt8 body[ 8 ];
    UInt32 size = CDRNWireWriter::EncodeVarUInt64( m_handle, body );
    m_component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_CLOSE, body, size );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::Fail( TDRNResourceStatus status, const CORE::CString& message, bool tellServer, bool discard, bool abortNow )
{GUCEF_TRACE;

    if ( !IsActive() )
        return;
    m_result = CDRNResourceTransferResult( status, message );
    if ( tellServer )
    {
        CancelRead();
        SendClose();
    }
    else
    {
        m_component->AddLinkOutstanding( m_peerId, -(Int64) m_readOutstanding );
        m_readOutstanding = 0;
        m_readId = 0;
        m_closeSent = true;
    }
    m_component->UpdateClientBuffered( -(Int64) m_bufferedBytes );
    m_bufferedBytes = 0;
    m_buffered = 0;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceTransfer: \"" << m_providerName << "/" << m_resourceName << "\" from member " << m_peerId << " failed: " <<
               CDRNResourceTransferResult::StatusToString( status ) << ", " << message );

    if ( m_pipe.IsNULL() )
    {
        FinishWith( TRANSFERSTATE_FAILED );
        return;
    }
    m_pipe->cancelled = true;
    if ( abortNow )
    {
        // the session closed or the member left: no tick of the session may come to collect a job, so the sink is
        // let go of here, after a job of it that may run (one batch of writes at most)
        {
            MT::CScopeMutex sinkLock( m_pipe->sinkLock );
            if ( m_pipe->state.IsValid() )
                m_sink->Abort( m_pipe->state, discard );
        }
        FinishWith( TRANSFERSTATE_FAILED );
        return;
    }

    // the sink is let go of on the pool, after a job of it that may run; the transfer ends once it was
    m_state = TRANSFERSTATE_ABORTING;
    m_job = CIDRNResourceIoJobPtr( GUCEF_NEW CAbortJob( m_pipe, discard ) );
    m_jobFuture = CDRNResourceIoTaskConsumer::Queue( m_component->m_settings.threadPoolName, m_job, m_component->GetSession().GetNode().GetWorkPulseGenerator() );
    if ( m_jobFuture.HasNoFuture() )
        FinishWith( TRANSFERSTATE_FAILED );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::FinishWith( TTransferState finalState )
{GUCEF_TRACE;

    m_state = finalState;
    m_job.Unlink();
    m_jobFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    if ( m_progressDue )
    {
        m_progressDue = false;
        CORE::TCloneableUInt64 progress( m_bytesDone );
        Emit( ProgressEvent, &progress );
    }
    if ( TRANSFERSTATE_COMPLETED == finalState )
    {
        ++m_component->m_stats.transfersCompleted;
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNResourceTransfer: \"" << m_providerName << "/" << m_resourceName << "\" from member " << m_peerId <<
                   " completed, " << m_bytesDone << " bytes, " << m_bytesReceived << " received" );
        Emit( CompletedEvent );
    }
    else
    {
        ++m_component->m_stats.transfersFailed;
        CDRNResourceTransferResultEventData eventData( m_result );
        Emit( FailedEvent, &eventData );
    }
    m_finished.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::FinishAbortNow( void )
{GUCEF_TRACE;

    // aborting, and no tick may come to see the abort job end (the session closed): the transfer ends once the sink is
    // free; the job lets go of the sink after this, which a resumable sink tolerates
    if ( TRANSFERSTATE_ABORTING != m_state )
        return;
    if ( !m_pipe.IsNULL() )
    {
        MT::CScopeMutex sinkLock( m_pipe->sinkLock );
    }
    FinishWith( TRANSFERSTATE_FAILED );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTransfer::Tick( UInt64 nodeNowMs )
{GUCEF_TRACE;

    if ( TRANSFERSTATE_COMPLETED == m_state || TRANSFERSTATE_FAILED == m_state )
        return;

    // a job of the sink that ended
    if ( !m_job.IsNULL() && m_jobFuture.IsReady() )
    {
        CIDRNResourceIoJobPtr job = m_job;
        m_job.Unlink();
        m_jobFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        if ( TRANSFERSTATE_ABORTING == m_state )
        {
            FinishWith( TRANSFERSTATE_FAILED );
            return;
        }
        TDRNResourceStatus status = DRN_RESSTATUS_OK;
        CORE::CString message;
        {
            MT::CScopeMutex sinkLock( m_pipe->sinkLock );
            status = m_pipe->jobStatus;
            message = m_pipe->jobMessage;
        }
        if ( TRANSFERSTATE_PREPARING == m_state )
        {
            if ( DRN_RESSTATUS_OK != status )
            {
                Fail( status, message, true );
                return;
            }
            {
                MT::CScopeMutex sinkLock( m_pipe->sinkLock );
                m_seen = m_pipe->state;
            }
            m_resumedChunks = 0;
            m_bytesResumed = 0;
            for ( UInt64 i=m_seen.FindFirstHeld( m_firstChunk ); i<m_endChunk; i=m_seen.FindFirstHeld( i+1 ) )
            {
                ++m_resumedChunks;
                m_bytesResumed += m_info.GetChunkBytes( i );
            }
            m_bytesDone = m_bytesResumed;
            m_progressDue = m_bytesResumed > 0;
            m_state = TRANSFERSTATE_TRANSFERRING;
            m_lastActivityMs = nodeNowMs;
        }
        else
        if ( TRANSFERSTATE_FINISHING == m_state )
        {
            if ( DRN_RESSTATUS_OK == status )
            {
                SendClose();
                FinishWith( TRANSFERSTATE_COMPLETED );
                return;
            }
            if ( DRN_RESSTATUS_DIGEST_MISMATCH == status && m_resumedChunks > 0 && !m_restartedFresh )
            {
                // what an earlier transfer left does not add up: once more from nothing, every chunk over the link
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceTransfer: \"" << m_providerName << "/" << m_resourceName << "\": " << message << ", starting over" );
                m_restartedFresh = true;
                m_strikes.clear();
                m_bytesDone = 0;
                StartPrepare( false );
                return;
            }
            Fail( status, message, true );
            return;
        }
    }

    if ( TRANSFERSTATE_OPENING == m_state )
    {
        if ( nodeNowMs >= m_startedMs + m_options.openTimeoutMs )
            Fail( DRN_RESSTATUS_TIMEOUT, "the serving side did not open the resource in time", true );
        return;
    }
    if ( TRANSFERSTATE_TRANSFERRING != m_state )
        return;

    if ( !m_drainFuture.HasNoFuture() && m_drainFuture.IsReady() )
    {
        CollectDrain();
        if ( !IsActive() )
            return;
    }

    if ( 0 == m_readId && !IsWindowComplete() )
        StartNextRead();

    if ( m_progressDue )
    {
        m_progressDue = false;
        CORE::TCloneableUInt64 progress( m_bytesDone );
        Emit( ProgressEvent, &progress );
    }

    if ( IsWindowComplete() && 0 == m_buffered && m_drainFuture.HasNoFuture() && !m_drainDue )
    {
        StartFinish();
        return;
    }

    if ( m_readOutstanding > 0 && nodeNowMs >= m_lastActivityMs + m_options.idleTimeoutMs )
        Fail( DRN_RESSTATUS_TIMEOUT, "no data arrived in time", true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
