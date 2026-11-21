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

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CIDRNPEERLIMITPOLICY_H
#include "gucefDRN_CIDRNPeerLimitPolicy.h"
#define GUCEF_DRN_CIDRNPEERLIMITPOLICY_H
#endif /* GUCEF_DRN_CIDRNPEERLIMITPOLICY_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#ifndef GUCEF_DRN_CDRNMEMORYRESOURCESINK_H
#include "gucefDRN_CDRNMemoryResourceSink.h"
#define GUCEF_DRN_CDRNMEMORYRESOURCESINK_H
#endif /* GUCEF_DRN_CDRNMEMORYRESOURCESINK_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#include "gucefDRN_CDRNResourceIoTaskConsumer.h"
#define GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACECODEC_H
#include "gucefDRN_CDRNNamespaceCodec.h"
#define GUCEF_DRN_CDRNNAMESPACECODEC_H
#endif /* GUCEF_DRN_CDRNNAMESPACECODEC_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETREESTORE_H
#include "gucefDRN_CDRNResourceTreeStore.h"
#define GUCEF_DRN_CDRNRESOURCETREESTORE_H
#endif /* GUCEF_DRN_CDRNRESOURCETREESTORE_H ? */

#include "gucefDRN_CDRNResourceSessionComponent.h"

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

/** Block size of a hash tree build reading the resource */
static const UInt32 TreeReadBlockSize = 16 * DRN_RESOURCE_CHUNK_SIZE;

/** How long what a CIDRNPeerLimitPolicy said about a member is kept before it is asked again */
static const UInt32 PeerLimitCacheMs = 2000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The lower of two limits where 0 is no maximum: the other one when one is 0 */
static UInt32
LowerLimit( UInt32 a, UInt32 b )
{GUCEF_TRACE;

    if ( 0 == a )
        return b;
    if ( 0 == b )
        return a;
    return a < b ? a : b;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNResourceSessionComponent::CProviderEntry
{
    public:

    CORE::CString name;
    CIDRNResourceProviderPtr provider;
    CDRNResourceShareRules rules;
};

/*-------------------------------------------------------------------------*/

/** A run of chunks read by a job, sent one by one */
class CReadRun
{
    public:

    UInt64 first;
    UInt64 count;
    UInt64 sent;
    UInt32 reservedBytes;
    CIDRNResourceIoJobPtr job;          /**< the read job, whose buffer holds the run */
    const CORE::CDynamicBuffer* data;

    CReadRun( void ) : first( 0 ), count( 0 ), sent( 0 ), reservedBytes( 0 ), job(), data( GUCEF_NULL ) {}
};

/*-------------------------------------------------------------------------*/

class CDRNResourceSessionComponent::CServeRead
{
    public:

    typedef GUCEF::vector< CReadRun* > TRunVector;

    UInt32 readId;
    UInt64 nextToRead;
    UInt64 end;
    UInt64 credit;                  /**< chunks the requester allows to be sent */
    CIDRNResourceIoJobPtr job;
    CORE::CFutureResult future;
    UInt64 jobFirst;
    UInt32 jobChunks;
    UInt32 jobReservedBytes;
    TRunVector runs;

    CServeRead( void )
        : readId( 0 ), nextToRead( 0 ), end( 0 ), credit( 0 ), job(), future( CORE::TASKSTATUS_UNDEFINED )
        , jobFirst( 0 ), jobChunks( 0 ), jobReservedBytes( 0 ), runs()
    {GUCEF_TRACE;
    }

    ~CServeRead()
    {GUCEF_TRACE;

        if ( !job.IsNULL() )
            job->RequestStop();
        for ( UInt32 i=0; i<runs.size(); ++i )
            GUCEF_DELETE runs[ i ];
        runs.clear();
    }

    /** Chunks read and not sent yet */
    UInt64 GetReadyChunks( void ) const
    {GUCEF_TRACE;

        UInt64 ready = 0;
        for ( UInt32 i=0; i<runs.size(); ++i )
            ready += runs[ i ]->count - runs[ i ]->sent;
        return ready;
    }

    /** Bytes held for the read: runs not sent and the job's reservation */
    UInt64 GetHeldBytes( void ) const
    {GUCEF_TRACE;

        UInt64 bytes = jobReservedBytes;
        for ( UInt32 i=0; i<runs.size(); ++i )
            bytes += runs[ i ]->reservedBytes;
        return bytes;
    }
};

/*-------------------------------------------------------------------------*/

class CDRNResourceSessionComponent::CServeHandle
{
    public:

    enum TState
    {
        STATE_RESOLVING     ,
        STATE_AWAITING_TREE ,
        STATE_BUILDING_TREE ,
        STATE_OPEN
    };

    typedef GUCEF::vector< CServeRead* > TReadVector;

    UInt32 peerId;
    UInt32 handle;
    CORE::CString providerName;
    CIDRNResourceProviderPtr provider;
    CORE::CString name;
    bool wantTree;
    CORE::CString expectedRevision;
    TState state;
    CIDRNResourceIoJobPtr job;
    CORE::CFutureResult future;
    CDRNResourceInfo info;
    TTreePtr tree;
    CORE::CString treeKey;
    TReadVector reads;
    UInt64 lastActivityMs;          /**< the open or the last read or credit, for the idle timeout */

    CServeHandle( void )
        : peerId( 0 ), handle( 0 ), providerName(), provider(), name(), wantTree( false ), expectedRevision()
        , state( STATE_RESOLVING ), job(), future( CORE::TASKSTATUS_UNDEFINED ), info(), tree(), treeKey(), reads()
        , lastActivityMs( CDRNClock::NowInMilliseconds() )
    {GUCEF_TRACE;
    }

    ~CServeHandle()
    {GUCEF_TRACE;

        if ( !job.IsNULL() )
            job->RequestStop();
        for ( UInt32 i=0; i<reads.size(); ++i )
            GUCEF_DELETE reads[ i ];
        reads.clear();
    }

    CServeRead* FindRead( UInt32 readId ) const
    {GUCEF_TRACE;

        for ( UInt32 i=0; i<reads.size(); ++i )
            if ( reads[ i ]->readId == readId )
                return reads[ i ];
        return GUCEF_NULL;
    }
};

/*-------------------------------------------------------------------------*/

/** Resolves a name with its provider, and asks for a tree the provider keeps */
class CDRNResourceSessionComponent::CResolveJob : public CIDRNResourceIoJob
{
    public:

    CIDRNResourceProviderPtr provider;
    CORE::CString name;
    bool wantTree;
    TDRNResourceStatus status;
    CDRNResourceInfo info;
    bool hasTree;
    TTreePtr tree;

    CResolveJob( void ) : CIDRNResourceIoJob(), provider(), name(), wantTree( false ), status( DRN_RESSTATUS_IO_ERROR ), info(), hasTree( false ), tree() {}

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        status = provider->Resolve( name, info );
        info.name = name;
        if ( DRN_RESSTATUS_OK != status || !wantTree )
            return;
        tree = TTreePtr( GUCEF_NEW CDRNResourceHashTree() );
        hasTree = provider->GetHashTree( name, info, *tree ) && tree->IsComplete() && tree->GetResourceSize() == info.size;
        if ( !hasTree )
            tree.Unlink();
    }
};

/*-------------------------------------------------------------------------*/

/** Builds the hash tree of a resource by reading it through its provider */
class CDRNResourceSessionComponent::CTreeJob : public CIDRNResourceIoJob
{
    public:

    CIDRNResourceProviderPtr provider;
    CORE::CString providerName;
    CORE::CString name;
    CDRNResourceInfo info;
    CORE::CString directory;        /**< the tree cache directory, empty for none */
    bool mayBuild;
    UInt64 maxDiskBytes;
    UInt32 maxAgeSeconds;
    TDRNResourceStatus status;
    TTreePtr tree;
    bool loaded;

    CTreeJob( void ) : CIDRNResourceIoJob(), provider(), providerName(), name(), info(), directory(), mayBuild( false ), maxDiskBytes( 0 ),
                       maxAgeSeconds( 0 ), status( DRN_RESSTATUS_IO_ERROR ), tree(), loaded( false ) {}

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        tree = TTreePtr( GUCEF_NEW CDRNResourceHashTree() );
        if ( !directory.IsNULLOrEmpty() && CDRNResourceTreeStore::Load( directory, providerName, info, *tree ) )
        {
            loaded = true;
            status = DRN_RESSTATUS_OK;
            return;
        }
        if ( !mayBuild )
        {
            status = DRN_RESSTATUS_TOO_LARGE;   // no tree: served without one
            return;
        }
        Build();
        if ( DRN_RESSTATUS_OK == status && !directory.IsNULLOrEmpty() )
        {
            if ( !CDRNResourceTreeStore::Save( directory, providerName, info, *tree ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceSessionComponent: Cannot keep the tree of \"" << providerName << "/" << name << "\" in \"" << directory << "\"" );
            CDRNResourceTreeStore::Prune( directory, maxDiskBytes, maxAgeSeconds );
        }
    }

    void Build( void )
    {GUCEF_TRACE;

        CDRNResourceHasher hasher;
        if ( !tree->Reset( info.size ) )
        {
            status = DRN_RESSTATUS_TOO_LARGE;
            return;
        }
        CORE::CDynamicBuffer block( TreeReadBlockSize, true );
        UInt64 chunk = 0;
        for ( UInt64 offset=0; offset<info.size; )
        {
            if ( IsStopRequested() )
            {
                status = DRN_RESSTATUS_CANCELLED;
                return;
            }
            UInt64 left = info.size - offset;
            UInt32 size = left < TreeReadBlockSize ? (UInt32) left : TreeReadBlockSize;
            UInt32 read = 0;
            status = provider->Read( name, info, offset, size, block.GetBufferPtr(), read );
            if ( DRN_RESSTATUS_OK != status )
                return;
            if ( read != size )
            {
                status = DRN_RESSTATUS_IO_ERROR;
                return;
            }
            const UInt8* bytes = static_cast< const UInt8* >( block.GetConstBufferPtr() );
            for ( UInt32 at=0; at<size; at+=DRN_RESOURCE_CHUNK_SIZE, ++chunk )
            {
                UInt32 chunkSize = size - at < DRN_RESOURCE_CHUNK_SIZE ? size - at : DRN_RESOURCE_CHUNK_SIZE;
                UInt8 leaf[ CDRNResourceHashTree::HashSize ];
                hasher.HashLeaf( bytes + at, chunkSize, leaf );
                tree->SetLeafHash( chunk, leaf );
            }
            offset += size;
        }
        status = tree->Finish( hasher ) ? DRN_RESSTATUS_OK : DRN_RESSTATUS_TOO_LARGE;
    }
};

/*-------------------------------------------------------------------------*/

/** Reads a run of chunks through the provider */
class CDRNResourceSessionComponent::CReadJob : public CIDRNResourceIoJob
{
    public:

    CIDRNResourceProviderPtr provider;
    CORE::CString name;
    CDRNResourceInfo info;
    UInt64 firstChunk;
    UInt32 chunkCount;
    TDRNResourceStatus status;
    CORE::CDynamicBuffer data;

    CReadJob( void ) : CIDRNResourceIoJob(), provider(), name(), info(), firstChunk( 0 ), chunkCount( 0 ), status( DRN_RESSTATUS_IO_ERROR ), data() {}

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        UInt64 offset = firstChunk * DRN_RESOURCE_CHUNK_SIZE;
        UInt64 end = ( firstChunk + chunkCount ) * DRN_RESOURCE_CHUNK_SIZE;
        if ( end > info.size )
            end = info.size;
        UInt32 size = end > offset ? (UInt32) ( end - offset ) : 0;
        if ( size > 0 && !data.SetBufferSize( size, false, true ) )
        {
            status = DRN_RESSTATUS_IO_ERROR;
            return;
        }
        data.SetDataSize( size );
        UInt32 read = 0;
        status = provider->Read( name, info, offset, size, data.GetBufferPtr(), read );
        if ( DRN_RESSTATUS_OK == status && read != size )
            status = DRN_RESSTATUS_IO_ERROR;
    }
};

/*-------------------------------------------------------------------------*/

/** What a member is to this side: its credit and rate (requesting), its turn and upload rate (serving) */
class CDRNResourceSessionComponent::CPeerBudget
{
    public:

    Int64 outstanding;
    CORE::CRateLimiter* limiter;
    UInt32 limiterRate;
    UInt32 limiterBurst;
    UInt32 serveTurn;
    CORE::CRateLimiter* uploadLimiter;      /**< serving: the upload rate of this member alone, NULL for none */
    UInt32 uploadLimiterRate;

    CPeerBudget( void ) : outstanding( 0 ), limiter( GUCEF_NULL ), limiterRate( 0 ), limiterBurst( 0 ), serveTurn( 0 ), uploadLimiter( GUCEF_NULL ), uploadLimiterRate( 0 ) {}

    ~CPeerBudget()
    {GUCEF_TRACE;

        GUCEF_DELETE limiter;
        limiter = GUCEF_NULL;
        GUCEF_DELETE uploadLimiter;
        uploadLimiter = GUCEF_NULL;
    }
};

/*-------------------------------------------------------------------------*/

/** The limits CIDRNPeerLimitPolicy gave a member, and until when they are kept */
class CDRNResourceSessionComponent::CPeerLimitEntry
{
    public:

    CDRNPeerLimits limits;
    UInt64 validUntilMs;

    CPeerLimitEntry( void ) : limits(), validUntilMs( 0 ) {}
};

/*-------------------------------------------------------------------------*/

class CDRNResourceSessionComponent::CCachedTree
{
    public:

    TTreePtr tree;
    UInt64 lastUseMs;
    UInt64 bytes;

    CCachedTree( void ) : tree(), lastUseMs( 0 ), bytes( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/** A listing of a member that is continued with a cursor: the open enumeration of a directory of a provider */
class CDRNResourceSessionComponent::CNsListing
{
    public:

    UInt32 peerId;
    UInt32 id;
    CORE::CString share;
    CORE::CString dir;
    CORE::CString dirRevision;
    CIDRNResourceProviderPtr provider;
    CIDRNNamespaceEnumeratorPtr enumerator;
    UInt32 nextSeq;                         /**< the sequence number the next request of the member has to carry */
    UInt64 lastActivityMs;
    bool busy;                              /**< a pool job works on its next page */
    bool atEnd;                             /**< the enumerator has no more entries */
    GUCEF::vector< CDRNNamespaceEntry > carry;   /**< entries read and accepted that did not fit the last page */

    CNsListing( void )
        : peerId( 0 ), id( 0 ), share(), dir(), dirRevision(), provider(), enumerator(), nextSeq( 0 )
        , lastActivityMs( CDRNClock::NowInMilliseconds() ), busy( false ), atEnd( false ), carry()
    {GUCEF_TRACE;
    }
};

/*-------------------------------------------------------------------------*/

/** The STAT of a batch of paths with a provider */
class CDRNResourceSessionComponent::CNsStatJob : public CIDRNResourceIoJob
{
    public:

    CIDRNResourceProviderPtr provider;
    GUCEF::vector< CORE::CString > paths;
    GUCEF::vector< UInt8 > todo;                        /**< 1: ask the provider; 0: the answer is already known */
    GUCEF::vector< CDRNNamespaceEntry > results;

    CNsStatJob( void ) : CIDRNResourceIoJob(), provider(), paths(), todo(), results() {}

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        for ( UInt32 i=0; i<paths.size(); ++i )
        {
            if ( 0 == todo[ i ] )
                continue;
            if ( IsStopRequested() )
                return;
            CDRNNamespaceEntry& entry = results[ i ];
            entry.Clear();
            TDRNResourceStatus status = provider->Stat( paths[ i ], entry );
            entry.status = status;
            entry.name = paths[ i ];
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Starts a listing (the directory is described, the enumerator opened) and/or reads the next entries of one */
class CDRNResourceSessionComponent::CNsListJob : public CIDRNResourceIoJob
{
    public:

    CIDRNResourceProviderPtr provider;
    CORE::CString dirName;
    CORE::CString ifRevisionNot;
    CIDRNNamespaceEnumeratorPtr enumerator;             /**< none: a new listing */
    UInt32 want;
    TDRNResourceStatus status;
    CDRNNamespaceEntry dirEntry;
    GUCEF::vector< CDRNNamespaceEntry > entries;
    bool atEnd;

    CNsListJob( void ) : CIDRNResourceIoJob(), provider(), dirName(), ifRevisionNot(), enumerator(), want( 0 ), status( DRN_RESSTATUS_IO_ERROR ), dirEntry(), entries(), atEnd( false ) {}

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( enumerator.IsNULL() )
        {
            status = provider->Stat( dirName, dirEntry );
            if ( DRN_RESSTATUS_OK != status )
                return;
            if ( !dirEntry.IsDirectory() )
            {
                status = DRN_RESSTATUS_NOT_A_DIRECTORY;
                return;
            }
            if ( !ifRevisionNot.IsNULLOrEmpty() && ifRevisionNot == dirEntry.revision )
            {
                status = DRN_RESSTATUS_UNCHANGED;
                return;
            }
            status = provider->OpenEnumerator( dirName, enumerator );
            if ( DRN_RESSTATUS_OK != status )
                return;
            if ( enumerator.IsNULL() )
            {
                status = DRN_RESSTATUS_IO_ERROR;
                return;
            }
        }
        if ( want > 0 )
            status = enumerator->ReadEntries( want, entries, atEnd );
        else
            status = DRN_RESSTATUS_OK;
    }
};

/*-------------------------------------------------------------------------*/

/** A namespace request of a member that a pool job works on */
class CDRNResourceSessionComponent::CNsServe
{
    public:

    UInt32 peerId;
    UInt32 requestId;
    UInt32 kind;
    CORE::CString share;
    CIDRNResourceProviderPtr provider;
    CIDRNResourceIoJobPtr job;
    CORE::CFutureResult future;
    CNsListing* listing;
    bool newListing;
    UInt32 maxEntries;
    CORE::CString dir;
    GUCEF::vector< CORE::CString > ifRevisions;                 /**< STAT: per path */
    GUCEF::vector< CDRNNamespaceEntry > presets;                /**< STAT: the paths that were answered without the provider */

    CNsServe( void )
        : peerId( 0 ), requestId( 0 ), kind( 0 ), share(), provider(), job(), future( CORE::TASKSTATUS_UNDEFINED ), listing( GUCEF_NULL )
        , newListing( false ), maxEntries( 0 ), dir(), ifRevisions(), presets()
    {GUCEF_TRACE;
    }

    ~CNsServe()
    {GUCEF_TRACE;

        if ( !job.IsNULL() )
            job->RequestStop();
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourceSessionComponent::CDRNResourceSessionComponent( CDRNSession& session )
    : CIDRNSessionComponent()
    , m_session( &session )
    , m_settings()
    , m_stats()
    , m_providers()
    , m_handles()
    , m_treeQueue()
    , m_treeBuilds( 0 )
    , m_treeCache()
    , m_treeCacheBytes( 0 )
    , m_uploadLimiter( GUCEF_NULL )
    , m_uploadLimiterRate( 0 )
    , m_peerLimitPolicy( GUCEF_NULL )
    , m_peerLimits()
    , m_transfers()
    , m_retired()
    , m_budgets()
    , m_clientHandles()
    , m_nextHandle( 1 )
    , m_creditRound( 0 )
    , m_txBuffer()
    , m_nsBuffer()
    , m_nsListings()
    , m_nsServing()
    , m_nsPending()
    , m_nextNsListingId( 1 )
    , m_nextNsRequestId( 1 )
{GUCEF_TRACE;

    CDRNResourceTransfer::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDRNResourceSessionComponent::~CDRNResourceSessionComponent()
{GUCEF_TRACE;

    // the handles are their callers': they stay, closed, without this component
    for ( UInt32 i=0; i<m_clientHandles.size(); ++i )
        m_clientHandles[ i ]->Detach();
    m_clientHandles.clear();

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        GUCEF_DELETE m_transfers[ i ];
    m_transfers.clear();
    for ( UInt32 i=0; i<m_retired.size(); ++i )
        GUCEF_DELETE m_retired[ i ];
    m_retired.clear();
    for ( UInt32 i=0; i<m_handles.size(); ++i )
        GUCEF_DELETE m_handles[ i ];
    m_handles.clear();
    DropNamespaceOf( 0, true );
    FailNamespaceRequestsOf( 0, true, DRN_RESSTATUS_LINK_LOST, "the session is gone" );
    m_treeQueue.clear();
    for ( TProviderMap::iterator i=m_providers.begin(); i!=m_providers.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_providers.clear();
    for ( TPeerBudgetMap::iterator i=m_budgets.begin(); i!=m_budgets.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_budgets.clear();
    for ( TTreeCache::iterator i=m_treeCache.begin(); i!=m_treeCache.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_treeCache.clear();
    GUCEF_DELETE m_uploadLimiter;
    m_uploadLimiter = GUCEF_NULL;
    DropPeerLimits( 0, true );
}

/*-------------------------------------------------------------------------*/

CDRNSession&
CDRNResourceSessionComponent::GetSession( void ) const
{GUCEF_TRACE;

    return *m_session;
}

/*-------------------------------------------------------------------------*/

UInt8
CDRNResourceSessionComponent::GetMessageClass( void ) const
{GUCEF_TRACE;

    return DRN_MSGCLASS_RESOURCE;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::SetSettings( const CDRNResourceTransferSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    m_settings = settings;
    DropPeerLimits( 0, true );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferSettings
CDRNResourceSessionComponent::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferStats
CDRNResourceSessionComponent::GetStats( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return m_stats;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::RegisterProvider( const CORE::CString& providerName, const CIDRNResourceProviderPtr& provider,
                                                const CDRNResourceShareRules& rules )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    if ( !CDRNResourceName::IsValidProviderName( providerName ) || provider.IsNULL() || !rules.IsValid() || m_providers.find( providerName ) != m_providers.end() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceSessionComponent: Cannot register a provider as \"" << providerName << "\"" );
        return false;
    }
    CProviderEntry* entry = GUCEF_NEW CProviderEntry();
    entry->name = providerName;
    entry->provider = provider;
    entry->rules = rules;
    m_providers[ providerName ] = entry;
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNResourceSessionComponent: Serving \"" << providerName << "\" (" << provider->GetDescription() << ") in session \"" <<
               m_session->GetName() << "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::UnregisterProvider( const CORE::CString& providerName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    TProviderMap::iterator i = m_providers.find( providerName );
    if ( i == m_providers.end() )
        return false;
    GUCEF_DELETE (*i).second;
    m_providers.erase( i );
    DropNamespaceOfProvider( providerName );

    // the handles of the provider end: reads under way are told, new reads find no handle
    for ( UInt32 h=0; h<m_handles.size(); )
    {
        CServeHandle* handle = m_handles[ h ];
        if ( handle->providerName != providerName )
        {
            ++h;
            continue;
        }
        if ( CServeHandle::STATE_OPEN == handle->state )
        {
            for ( UInt32 r=0; r<handle->reads.size(); ++r )
                SendReadEnd( handle->peerId, handle->handle, handle->reads[ r ]->readId, DRN_RESSTATUS_CLOSED, "the provider is no longer served" );
        }
        else
        {
            SendInfo( handle->peerId, handle->handle, DRN_RESSTATUS_CLOSED, "the provider is no longer served", GUCEF_NULL );
        }
        DropHandle( handle );
    }
    StartTreeBuilds();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::HasProvider( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return m_providers.find( providerName ) != m_providers.end();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetOpenHandleCount( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return 0 == peerId ? (UInt32) m_handles.size() : CountHandles( peerId );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetActiveReadCount( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return 0 == peerId ? CountAllReads() : CountReads( peerId );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceSessionComponent::GetBufferedBytes( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return 0 == peerId ? m_stats.serverBufferedBytes : CountBufferedBytes( peerId );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNamespaceServingCount( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return 0 == peerId ? (UInt32) m_nsServing.size() : CountNsServing( peerId );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::SetPeerLimitPolicy( CIDRNPeerLimitPolicy* policy )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    m_peerLimitPolicy = policy;
    DropPeerLimits( 0, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::RefreshPeerLimits( UInt32 peerId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    DropPeerLimits( peerId, 0 == peerId );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropPeerLimits( UInt32 peerId, bool allPeers )
{GUCEF_TRACE;

    if ( allPeers )
    {
        for ( TPeerLimitMap::iterator i=m_peerLimits.begin(); i!=m_peerLimits.end(); ++i )
            GUCEF_DELETE (*i).second;
        m_peerLimits.clear();
        return;
    }
    TPeerLimitMap::iterator i = m_peerLimits.find( peerId );
    if ( i != m_peerLimits.end() )
    {
        GUCEF_DELETE (*i).second;
        m_peerLimits.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

CDRNPeerLimits
CDRNResourceSessionComponent::GetConfiguredPeerLimits( void ) const
{GUCEF_TRACE;

    CDRNPeerLimits limits;
    limits.maxOpenHandles = m_settings.maxOpenHandlesPerPeer;
    limits.maxActiveReads = m_settings.maxActiveReadsPerPeer;
    limits.maxNamespaceRequests = m_settings.maxNamespaceRequestsPerPeer;
    limits.maxNamespaceListings = m_settings.maxNamespaceListingsPerPeer;
    limits.maxUploadBytesPerSecond = m_settings.maxUploadBytesPerSecondPerPeer;
    limits.maxBufferedBytes = m_settings.maxBufferedBytesPerPeer;
    return limits;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLimits
CDRNResourceSessionComponent::GetPeerLimits( UInt32 peerId )
{GUCEF_TRACE;

    // no policy: the configured values, nothing is kept. With one: what it said, for a short time per member
    if ( GUCEF_NULL == m_peerLimitPolicy )
        return GetConfiguredPeerLimits();

    UInt64 nowMs = CDRNClock::NowInMilliseconds();
    TPeerLimitMap::iterator i = m_peerLimits.find( peerId );
    if ( i != m_peerLimits.end() && nowMs < (*i).second->validUntilMs )
        return (*i).second->limits;

    CDRNPeerLimits configured = GetConfiguredPeerLimits();
    CDRNPeerLimits effective = configured;
    m_peerLimitPolicy->GetEffectiveLimits( peerId, m_session->GetMemberAuthResult( peerId ), configured, effective );
    CPeerLimitEntry* entry = GUCEF_NULL;
    if ( i != m_peerLimits.end() )
    {
        entry = (*i).second;
    }
    else
    {
        entry = GUCEF_NEW CPeerLimitEntry();
        m_peerLimits[ peerId ] = entry;
    }
    entry->limits = effective;
    entry->validUntilMs = nowMs + PeerLimitCacheMs;
    return effective;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLimits
CDRNResourceSessionComponent::GetEffectivePeerLimits( UInt32 peerId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return GetPeerLimits( peerId );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::IsLimitReached( UInt32 serverCount, UInt32 serverMax, UInt32 peerCount, UInt32 peerMax, const char* what, CORE::CString& why )
{GUCEF_TRACE;

    // two independent limits, whichever is reached first refuses. The member's own is named first: it is the one the
    // member can do something about
    if ( 0 != peerMax && peerCount >= peerMax )
    {
        ++m_stats.refusedByMemberLimit;
        why = "the per-member limit of " + CORE::ToString( peerMax ) + " " + what + " is reached";
        return true;
    }
    if ( 0 != serverMax && serverCount >= serverMax )
    {
        ++m_stats.refusedByServerLimit;
        why = "the server-wide limit of " + CORE::ToString( serverMax ) + " " + what + " is reached";
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLinkPtr
CDRNResourceSessionComponent::GetUsableLink( UInt32 peerId, TDRNResourceStatus& status, CORE::CString& why, UInt8 minimumMinor ) const
{GUCEF_TRACE;

    status = DRN_RESSTATUS_OK;
    if ( !m_session->IsActive() || !m_session->HasMember( peerId ) || peerId == m_session->GetLocalPeerId() )
    {
        status = DRN_RESSTATUS_LINK_LOST;
        why = "not a member of the session";
        return CDRNPeerLinkPtr();
    }
    CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
    if ( link.IsNULL() )
    {
        status = DRN_RESSTATUS_UNSUPPORTED;
        why = "no direct link to the member";
        return CDRNPeerLinkPtr();
    }
    if ( !link->IsOperational() )
    {
        status = DRN_RESSTATUS_LINK_LOST;
        why = "the link to the member is down";
        return CDRNPeerLinkPtr();
    }
    if ( link->GetRemoteProtocolMinor() < minimumMinor )
    {
        status = DRN_RESSTATUS_UNSUPPORTED;
        why = DRN_PROTOCOL_MINOR_RESOURCE == minimumMinor ? "the member's node has no resource transfers (protocol 2." + CORE::ToString( (UInt32) link->GetRemoteProtocolMinor() ) + ")"
                                                          : "the member's node has no namespace operations (protocol 2." + CORE::ToString( (UInt32) link->GetRemoteProtocolMinor() ) + ")";
        return CDRNPeerLinkPtr();
    }
    if ( !link->IsProtected() && !m_settings.allowUnprotectedLinks )
    {
        status = DRN_RESSTATUS_INSECURE_LINK;
        why = "the link has no packet protection";
        return CDRNPeerLinkPtr();
    }
    return link;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::Send( UInt32 peerId, UInt8 lane, UInt32 opcode, const void* part1, UInt32 size1, const void* part2, UInt32 size2 )
{GUCEF_TRACE;

    CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
    if ( link.IsNULL() || !link->IsOperational() )
        return false;
    return link->SendGather( lane, DRN_MSGCLASS_RESOURCE, opcode, part1, size1, part2, size2 );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransfer*
CDRNResourceSessionComponent::StartTransfer( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                             bool wholeResource, UInt64 offset, UInt64 length, const CIDRNResourceSinkPtr& sink,
                                             const CDRNResourceTransferOptions& options )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    UInt32 handle = m_nextHandle++;
    if ( 0 == m_nextHandle )
        m_nextHandle = 1;
    CDRNResourceTransfer* transfer = GUCEF_NEW CDRNResourceTransfer( *this, peerId, handle, providerName, resourceName, wholeResource, offset, length, sink, options );
    m_transfers.push_back( transfer );

    // names are checked here as well, so a mistake does not cost a round trip; the transfer starts on the next tick
    // of the node, after the caller could subscribe to it
    if ( !CDRNResourceName::IsValidProviderName( providerName ) || !CDRNResourceName::IsValidResourceName( resourceName ) )
        transfer->m_result = CDRNResourceTransferResult( DRN_RESSTATUS_INVALID_NAME, "not a valid provider or resource name" );
    return transfer;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransfer*
CDRNResourceSessionComponent::Fetch( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                     const CIDRNResourceSinkPtr& sink, const CDRNResourceTransferOptions& options )
{GUCEF_TRACE;

    return StartTransfer( peerId, providerName, resourceName, true, 0, 0, sink, options );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransfer*
CDRNResourceSessionComponent::Read( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                    UInt64 offset, UInt64 length, const CIDRNResourceSinkPtr& sink, const CDRNResourceTransferOptions& options )
{GUCEF_TRACE;

    return StartTransfer( peerId, providerName, resourceName, false, offset, length, sink, options );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceSessionComponent::ReadRange( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                         UInt64 offset, UInt64 length, CORE::CDynamicBuffer& data,
                                         const CDRNResourceTransferOptions& options, UInt32 timeoutInMs )
{GUCEF_TRACE;

    data.SetDataSize( 0 );
    if ( length > 0x7FFFFFFF )
        return CDRNResourceTransferResult( DRN_RESSTATUS_TOO_LARGE, "a window of more than 2 GiB does not fit in memory at once" );
    if ( m_session->GetNode().IsWorkingOnThisThread() )
        return CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "a wait on a thread that works on the node would never end" );

    CDRNMemoryResourceSink* memorySink = GUCEF_NEW CDRNMemoryResourceSink( (UInt32) length );
    CIDRNResourceSinkPtr sink( memorySink );
    CDRNResourceTransfer* transfer = Read( peerId, providerName, resourceName, offset, length, sink, options );
    if ( !transfer->WaitForCompletion( timeoutInMs ) )
    {
        transfer->Cancel();
        if ( !transfer->WaitForCompletion( 5000 ) )
        {
            // the node did not get to it: the component lets go of it once it is done
            MT::CScopeMutex lock( m_session->GetNode().GetLock() );
            transfer->m_releaseWhenFinished = true;
            return CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "the read did not complete in time" );
        }
        Release( transfer );
        return CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "the read did not complete in time" );
    }
    CDRNResourceTransferResult result = transfer->GetResult();
    if ( result.IsOk() )
        memorySink->TakeData( data );
    Release( transfer );
    return result;
}

/*-------------------------------------------------------------------------*/

CDRNResourceHandlePtr
CDRNResourceSessionComponent::OpenHandle( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                          const CDRNResourceTransferOptions& options )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    UInt32 handleId = m_nextHandle++;
    if ( 0 == m_nextHandle )
        m_nextHandle = 1;
    CDRNResourceHandle* handle = GUCEF_NEW CDRNResourceHandle( *this, peerId, handleId, providerName, resourceName, options );
    CDRNResourceHandlePtr handlePtr( handle );

    TDRNResourceStatus status = DRN_RESSTATUS_OK;
    CORE::CString why;
    if ( !CDRNResourceName::IsValidProviderName( providerName ) || !CDRNResourceName::IsValidResourceName( resourceName ) )
    {
        status = DRN_RESSTATUS_INVALID_NAME;
        why = "not a valid provider or resource name";
    }
    else
    if ( !GetUsableLink( peerId, status, why ).IsNULL() )
    {
        m_clientHandles.push_back( handle );
        if ( handle->SendOpen() )
        {
            if ( m_session->GetNode().IsRunningOwnThread() )
                m_session->GetNode().WakeOwnThread();
            return handlePtr;
        }
        m_clientHandles.pop_back();
        status = DRN_RESSTATUS_LINK_LOST;
        why = "the open could not be sent";
    }
    handle->OnLost( status, why );
    handle->Detach();
    return handlePtr;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetClientHandleCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return (UInt32) m_clientHandles.size();
}

/*-------------------------------------------------------------------------*/

CDRNResourceHandle*
CDRNResourceSessionComponent::FindClientHandle( UInt32 peerId, UInt32 handle ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_clientHandles.size(); ++i )
        if ( m_clientHandles[ i ]->m_handleId == handle && m_clientHandles[ i ]->m_peerId == peerId )
            return m_clientHandles[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::ForgetHandle( CDRNResourceHandle* handle )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_clientHandles.size(); ++i )
    {
        if ( m_clientHandles[ i ] == handle )
        {
            m_clientHandles.erase( m_clientHandles.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::LoseClientHandles( UInt32 peerId, bool allPeers, TDRNResourceStatus status, const CORE::CString& why )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_clientHandles.size(); )
    {
        CDRNResourceHandle* handle = m_clientHandles[ i ];
        if ( !allPeers && handle->m_peerId != peerId )
        {
            ++i;
            continue;
        }
        handle->OnLost( status, why );
        handle->Detach();
        m_clientHandles.erase( m_clientHandles.begin() + i );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::Release( CDRNResourceTransfer* transfer )
{GUCEF_TRACE;

    bool released = false;
    {
        MT::CScopeMutex lock( m_session->GetNode().GetLock() );
        for ( UInt32 i=0; i<m_transfers.size(); ++i )
        {
            if ( m_transfers[ i ] == transfer )
            {
                if ( CDRNResourceTransfer::TRANSFERSTATE_COMPLETED != transfer->m_state && CDRNResourceTransfer::TRANSFERSTATE_FAILED != transfer->m_state )
                    return false;
                m_transfers.erase( m_transfers.begin() + i );
                released = true;
                break;
            }
        }
    }

    // without the node lock: the transfer lets go of its mail (CDRNApplicationNotifier)
    if ( released )
        GUCEF_DELETE transfer;
    return released;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::Reset( void )
{GUCEF_TRACE;

    // under the node lock, as the bulk transfers: the transfers of an earlier run go with the component
    DropHandlesOf( 0, true );
    DropNamespaceOf( 0, true );
    FailNamespaceRequestsOf( 0, true, DRN_RESSTATUS_LINK_LOST, "a new run of the session" );
    FailTransfersOf( 0, true, DRN_RESSTATUS_LINK_LOST, "a new run of the session" );
    LoseClientHandles( 0, true, DRN_RESSTATUS_LINK_LOST, "a new run of the session" );
    m_retired.insert( m_retired.end(), m_transfers.begin(), m_transfers.end() );
    m_transfers.clear();
    for ( TPeerBudgetMap::iterator i=m_budgets.begin(); i!=m_budgets.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_budgets.clear();
    DropPeerLimits( 0, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnMemberLeft( CDRNSession& /* session */, UInt32 peerId )
{GUCEF_TRACE;

    DropHandlesOf( peerId, false );
    DropNamespaceOf( peerId, false );
    FailNamespaceRequestsOf( peerId, false, DRN_RESSTATUS_LINK_LOST, "the member left" );
    FailTransfersOf( peerId, false, DRN_RESSTATUS_LINK_LOST, "the member left" );
    LoseClientHandles( peerId, false, DRN_RESSTATUS_LINK_LOST, "the member left" );
    TPeerBudgetMap::iterator i = m_budgets.find( peerId );
    if ( i != m_budgets.end() )
    {
        GUCEF_DELETE (*i).second;
        m_budgets.erase( i );
    }
    DropPeerLimits( peerId, false );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnSessionClosed( CDRNSession& /* session */ )
{GUCEF_TRACE;

    DropHandlesOf( 0, true );
    DropNamespaceOf( 0, true );
    FailNamespaceRequestsOf( 0, true, DRN_RESSTATUS_LINK_LOST, "the session closed" );
    FailTransfersOf( 0, true, DRN_RESSTATUS_LINK_LOST, "the session closed" );
    LoseClientHandles( 0, true, DRN_RESSTATUS_LINK_LOST, "the session closed" );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::FailTransfersOf( UInt32 peerId, bool allPeers, TDRNResourceStatus status, const CORE::CString& why )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
    {
        CDRNResourceTransfer* transfer = m_transfers[ i ];
        if ( !allPeers && transfer->m_peerId != peerId )
            continue;
        if ( CDRNResourceTransfer::TRANSFERSTATE_ABORTING == transfer->m_state )
        {
            transfer->FinishAbortNow();
            continue;
        }
        transfer->Fail( status, why, false, false, true );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnSessionMessage( CDRNSession& /* session */, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    // direct links only: the member at the other end of the link speaks for itself
    if ( 0 == fromPeerId || fromPeerId != link.GetSessionPeerId() )
    {
        ++m_stats.droppedMessages;
        return;
    }
    if ( message.opcode >= DRN_RES_NS_HELLO )
    {
        OnNamespaceMessage( fromPeerId, link, message );
        return;
    }
    CDRNWireReader reader( message.payload );
    switch ( message.opcode )
    {
        case DRN_RES_OPEN:      { OnOpen( fromPeerId, link, reader ); break; }
        case DRN_RES_READ:      { OnRead( fromPeerId, reader ); break; }
        case DRN_RES_CREDIT:    { OnCredit( fromPeerId, reader ); break; }
        case DRN_RES_CANCEL:    { OnCancel( fromPeerId, reader ); break; }
        case DRN_RES_CLOSE:     { OnClose( fromPeerId, reader ); break; }
        case DRN_RES_INFO:      { OnInfo( fromPeerId, reader ); break; }
        case DRN_RES_DATA:      { OnData( fromPeerId, reader ); break; }
        case DRN_RES_READ_END:  { OnReadEnd( fromPeerId, reader ); break; }
        default:                { ++m_stats.droppedMessages; break; }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnSessionTick( CDRNSession& /* session */, UInt64 nodeNow )
{GUCEF_TRACE;

    UInt64 nowMs = nodeNow / 1000;
    TickServing( nowMs );
    TickNamespace( nowMs );

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
    {
        CDRNResourceTransfer* transfer = m_transfers[ i ];
        if ( CDRNResourceTransfer::TRANSFERSTATE_OPENING == transfer->m_state && 0 == transfer->m_startedMs )
        {
            if ( !transfer->m_result.IsOk() )
                transfer->Fail( transfer->m_result.status, transfer->m_result.message, false );
            else
                transfer->Start( nowMs );
            continue;
        }
        transfer->Tick( nowMs );
    }
    ShareCredit( nowMs );

    for ( UInt32 i=0; i<m_transfers.size(); )
    {
        CDRNResourceTransfer* transfer = m_transfers[ i ];
        if ( transfer->m_releaseWhenFinished && !transfer->IsActive() && CDRNResourceTransfer::TRANSFERSTATE_ABORTING != transfer->m_state )
        {
            m_retired.push_back( transfer );
            m_transfers.erase( m_transfers.begin() + i );
            continue;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/
//  Serving
/*-------------------------------------------------------------------------*/

CDRNResourceSessionComponent::CServeHandle*
CDRNResourceSessionComponent::FindHandle( UInt32 peerId, UInt32 handle ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_handles.size(); ++i )
        if ( m_handles[ i ]->peerId == peerId && m_handles[ i ]->handle == handle )
            return m_handles[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::CountHandles( UInt32 peerId ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_handles.size(); ++i )
        if ( m_handles[ i ]->peerId == peerId )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::CountReads( UInt32 peerId ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_handles.size(); ++i )
        if ( m_handles[ i ]->peerId == peerId )
            count += (UInt32) m_handles[ i ]->reads.size();
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::CountAllReads( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_handles.size(); ++i )
        count += (UInt32) m_handles[ i ]->reads.size();
    return count;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceSessionComponent::CountBufferedBytes( UInt32 peerId ) const
{GUCEF_TRACE;

    UInt64 bytes = 0;
    for ( UInt32 i=0; i<m_handles.size(); ++i )
    {
        if ( m_handles[ i ]->peerId != peerId )
            continue;
        for ( UInt32 r=0; r<m_handles[ i ]->reads.size(); ++r )
            bytes += m_handles[ i ]->reads[ r ]->GetHeldBytes();
    }
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::UpdateServerBuffered( Int64 delta )
{GUCEF_TRACE;

    Int64 value = (Int64) m_stats.serverBufferedBytes + delta;
    m_stats.serverBufferedBytes = value < 0 ? 0 : (UInt64) value;
    if ( m_stats.serverBufferedBytes > m_stats.peakServerBufferedBytes )
        m_stats.peakServerBufferedBytes = m_stats.serverBufferedBytes;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropHandle( CServeHandle* handle )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_handles.size(); ++i )
    {
        if ( m_handles[ i ] == handle )
        {
            m_handles.erase( m_handles.begin() + i );
            break;
        }
    }
    for ( UInt32 i=0; i<m_treeQueue.size(); ++i )
    {
        if ( m_treeQueue[ i ] == handle )
        {
            m_treeQueue.erase( m_treeQueue.begin() + i );
            break;
        }
    }
    if ( CServeHandle::STATE_BUILDING_TREE == handle->state && m_treeBuilds > 0 )
        --m_treeBuilds;
    for ( UInt32 r=0; r<handle->reads.size(); ++r )
        UpdateServerBuffered( -(Int64) handle->reads[ r ]->GetHeldBytes() );
    GUCEF_DELETE handle;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropHandlesOf( UInt32 peerId, bool allPeers )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_handles.size(); )
    {
        if ( allPeers || m_handles[ i ]->peerId == peerId )
        {
            DropHandle( m_handles[ i ] );
            continue;
        }
        ++i;
    }
    StartTreeBuilds();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::SendInfo( UInt32 peerId, UInt32 handle, TDRNResourceStatus status, const CORE::CString& message, const CDRNResourceInfo* info )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( handle );
    writer.WriteVarUInt32( (UInt32) status );
    writer.WriteString( message );
    if ( DRN_RESSTATUS_OK == status && GUCEF_NULL != info )
    {
        writer.WriteVarUInt64( info->size );
        writer.WriteVarUInt64( info->modifiedUtcMs );
        writer.WriteString( info->revision );
        writer.WriteVarUInt32( DRN_RESOURCE_CHUNK_SIZE );
        UInt8 flags = ( info->hasSha256 ? DRN_RES_DIGEST_SHA256 : 0 ) | ( info->hasTreeRoot ? DRN_RES_DIGEST_TREE : 0 );
        writer.WriteUInt8( flags );
        if ( info->hasSha256 )
            writer.WriteBytes( info->sha256, CDRNResourceInfo::DigestSize );
        if ( info->hasTreeRoot )
            writer.WriteBytes( info->treeRoot, CDRNResourceInfo::DigestSize );
        ++m_stats.opensServed;
    }
    else
    {
        ++m_stats.opensRefused;
    }
    Send( peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_INFO, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::SendReadEnd( UInt32 peerId, UInt32 handle, UInt32 readId, TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( handle );
    writer.WriteVarUInt32( readId );
    writer.WriteVarUInt32( (UInt32) status );
    writer.WriteString( message );
    Send( peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_READ_END, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
    ++m_stats.readsFailed;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnOpen( UInt32 peerId, CDRNPeerLink& link, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handleId = 0;
    CORE::CString providerName;
    CORE::CString name;
    UInt32 flags = 0;
    CORE::CString expectedRevision;
    if ( !reader.ReadVarUInt32( handleId ) || !reader.ReadString( providerName ) || !reader.ReadString( name ) ||
         !reader.ReadVarUInt32( flags ) || !reader.ReadString( expectedRevision ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    if ( GUCEF_NULL != FindHandle( peerId, handleId ) )
    {
        SendInfo( peerId, handleId, DRN_RESSTATUS_PROTOCOL_ERROR, "the handle is in use", GUCEF_NULL );
        return;
    }

    // in this order, so that a refusal tells nothing about whether the name exists
    if ( !link.IsProtected() && !m_settings.allowUnprotectedLinks )
    {
        SendInfo( peerId, handleId, DRN_RESSTATUS_INSECURE_LINK, "resources are served over protected links only", GUCEF_NULL );
        return;
    }
    CORE::CString busyWhy;
    CDRNPeerLimits peerLimits = GetPeerLimits( peerId );
    if ( IsLimitReached( (UInt32) m_handles.size(), m_settings.maxOpenHandles, 0 != peerLimits.maxOpenHandles ? CountHandles( peerId ) : 0, peerLimits.maxOpenHandles,
                         "open resources", busyWhy ) )
    {
        SendInfo( peerId, handleId, DRN_RESSTATUS_BUSY, busyWhy, GUCEF_NULL );
        return;
    }
    TProviderMap::iterator p = m_providers.find( providerName );
    if ( p == m_providers.end() )
    {
        SendInfo( peerId, handleId, DRN_RESSTATUS_NO_SUCH_PROVIDER, "no such provider", GUCEF_NULL );
        return;
    }
    CProviderEntry& entry = *(*p).second;
    if ( !CDRNResourceName::IsValidResourceName( name ) )
    {
        SendInfo( peerId, handleId, DRN_RESSTATUS_INVALID_NAME, "not a valid resource name", GUCEF_NULL );
        return;
    }

    // nothing is readable unless the application shared it (the provider's share rules); the access policy has the
    // last word. Decided before the name is resolved: a refusal tells nothing about whether it exists
    const CDRNAuthResult& auth = m_session->GetMemberAuthResult( peerId );
    bool shared = entry.rules.Allows( auth.role, name );
    CORE::CString refusal;
    bool retryLater = false;
    if ( !m_session->GetNode().GetAccessPolicy().DecideReadResourceAs( peerId, auth, providerName, name, shared, refusal, retryLater ) )
    {
        if ( retryLater )
        {
            // a condition that mends by itself: busy, as the limits are, so that the client asks again instead of giving up
            SendInfo( peerId, handleId, DRN_RESSTATUS_BUSY, refusal.IsNULLOrEmpty() ? CORE::CString( "not ready, ask again in a moment" ) : refusal, GUCEF_NULL );
            return;
        }
        SendInfo( peerId, handleId, DRN_RESSTATUS_FORBIDDEN, refusal.IsNULLOrEmpty() ? CORE::CString( "not shared with this member" ) : refusal, GUCEF_NULL );
        return;
    }
    if ( IsHiddenByPolicy( peerId, auth, providerName, name, false ) )
    {
        // an entry the policy hides does not exist for this member
        SendInfo( peerId, handleId, DRN_RESSTATUS_NOT_FOUND, "no such resource", GUCEF_NULL );
        return;
    }

    CServeHandle* handle = GUCEF_NEW CServeHandle();
    handle->peerId = peerId;
    handle->handle = handleId;
    handle->providerName = providerName;
    handle->provider = entry.provider;
    handle->name = name;
    handle->wantTree = 0 != ( flags & DRN_RES_OPEN_FLAG_TREE );
    handle->expectedRevision = expectedRevision;
    handle->state = CServeHandle::STATE_RESOLVING;

    CResolveJob* job = GUCEF_NEW CResolveJob();
    job->provider = entry.provider;
    job->name = name;
    job->wantTree = handle->wantTree;
    handle->job = CIDRNResourceIoJobPtr( job );
    handle->future = CDRNResourceIoTaskConsumer::Queue( m_settings.threadPoolName, handle->job, m_session->GetNode().GetWorkPulseGenerator() );
    if ( handle->future.HasNoFuture() )
    {
        GUCEF_DELETE handle;
        SendInfo( peerId, handleId, DRN_RESSTATUS_BUSY, "no thread pool to resolve on", GUCEF_NULL );
        return;
    }
    m_handles.push_back( handle );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnResolved( CServeHandle& handle )
{GUCEF_TRACE;

    CResolveJob* job = static_cast< CResolveJob* >( handle.job.GetPointerAlways() );
    TDRNResourceStatus status = job->status;
    CDRNResourceInfo info = job->info;
    bool hasTree = job->hasTree;
    TTreePtr tree = job->tree;
    handle.job.Unlink();
    handle.future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );

    if ( DRN_RESSTATUS_OK != status )
    {
        SendInfo( handle.peerId, handle.handle, status, DRN_RESSTATUS_NOT_FOUND == status ? "no such resource" : "the resource cannot be served", GUCEF_NULL );
        DropHandle( &handle );
        return;
    }
    if ( !handle.expectedRevision.IsNULLOrEmpty() && handle.expectedRevision != info.revision )
    {
        SendInfo( handle.peerId, handle.handle, DRN_RESSTATUS_CHANGED, "the resource is another revision", GUCEF_NULL );
        DropHandle( &handle );
        return;
    }
    handle.info = info;
    if ( handle.wantTree && hasTree )
    {
        handle.tree = tree;
        handle.info.hasTreeRoot = handle.tree->GetRoot( handle.info.treeRoot );
    }
    if ( handle.wantTree && !handle.info.hasTreeRoot )
    {
        ObtainTree( handle );
        return;
    }
    handle.state = CServeHandle::STATE_OPEN;
    handle.lastActivityMs = CDRNClock::NowInMilliseconds();
    SendInfo( handle.peerId, handle.handle, DRN_RESSTATUS_OK, CORE::CString::Empty, &handle.info );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::ObtainTree( CServeHandle& handle )
{GUCEF_TRACE;

    handle.treeKey = handle.providerName + "\n" + handle.name + "\n" + handle.info.revision;
    TTreeCache::iterator cached = m_treeCache.find( handle.treeKey );
    if ( cached != m_treeCache.end() )
    {
        (*cached).second->lastUseMs = CDRNClock::NowInMilliseconds();
        handle.tree = (*cached).second->tree;
        handle.info.hasTreeRoot = handle.tree->GetRoot( handle.info.treeRoot );
        handle.state = CServeHandle::STATE_OPEN;
        handle.lastActivityMs = CDRNClock::NowInMilliseconds();
        SendInfo( handle.peerId, handle.handle, DRN_RESSTATUS_OK, CORE::CString::Empty, &handle.info );
        return;
    }
    bool mayBuild = 0 != m_settings.maxConcurrentTreeBuilds && handle.info.size <= m_settings.maxTreeResourceBytes;
    if ( !mayBuild && m_settings.treeCacheDirectory.IsNULLOrEmpty() )
    {
        // served without a tree: the requester decides whether that will do
        handle.state = CServeHandle::STATE_OPEN;
        handle.lastActivityMs = CDRNClock::NowInMilliseconds();
        SendInfo( handle.peerId, handle.handle, DRN_RESSTATUS_OK, CORE::CString::Empty, &handle.info );
        return;
    }
    handle.state = CServeHandle::STATE_AWAITING_TREE;
    m_treeQueue.push_back( &handle );
    StartTreeBuilds();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::StartTreeBuilds( void )
{GUCEF_TRACE;

    // a tree cache directory gives one slot to load trees, also when none are built
    UInt32 slots = m_settings.maxConcurrentTreeBuilds;
    if ( 0 == slots && !m_settings.treeCacheDirectory.IsNULLOrEmpty() )
        slots = 1;
    for ( UInt32 q=0; q<m_treeQueue.size() && m_treeBuilds<slots; )
    {
        CServeHandle* handle = m_treeQueue[ q ];
        bool beingBuilt = false;
        for ( UInt32 i=0; i<m_handles.size() && !beingBuilt; ++i )
            beingBuilt = CServeHandle::STATE_BUILDING_TREE == m_handles[ i ]->state && m_handles[ i ]->treeKey == handle->treeKey;
        if ( beingBuilt )
        {
            // the build under way serves this one as well
            ++q;
            continue;
        }
        m_treeQueue.erase( m_treeQueue.begin() + q );

        CTreeJob* job = GUCEF_NEW CTreeJob();
        job->provider = handle->provider;
        job->providerName = handle->providerName;
        job->name = handle->name;
        job->info = handle->info;
        job->directory = m_settings.treeCacheDirectory;
        job->mayBuild = 0 != m_settings.maxConcurrentTreeBuilds && handle->info.size <= m_settings.maxTreeResourceBytes;
        job->maxDiskBytes = m_settings.treeCacheMaxDiskBytes;
        job->maxAgeSeconds = m_settings.treeCacheMaxAgeSeconds;
        handle->job = CIDRNResourceIoJobPtr( job );
        handle->future = CDRNResourceIoTaskConsumer::Queue( m_settings.threadPoolName, handle->job, m_session->GetNode().GetWorkPulseGenerator() );
        if ( handle->future.HasNoFuture() )
        {
            handle->job.Unlink();
            handle->state = CServeHandle::STATE_OPEN;
            handle->lastActivityMs = CDRNClock::NowInMilliseconds();
            SendInfo( handle->peerId, handle->handle, DRN_RESSTATUS_OK, CORE::CString::Empty, &handle->info );
            continue;
        }
        handle->state = CServeHandle::STATE_BUILDING_TREE;
        ++m_treeBuilds;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::CacheTree( const CORE::CString& key, const TTreePtr& tree, UInt64 nowMs )
{GUCEF_TRACE;

    UInt64 bytes = tree->GetMemoryBytes();
    if ( bytes > m_settings.maxTreeCacheBytes )
        return;
    while ( m_treeCacheBytes + bytes > m_settings.maxTreeCacheBytes && !m_treeCache.empty() )
    {
        TTreeCache::iterator oldest = m_treeCache.begin();
        for ( TTreeCache::iterator i=m_treeCache.begin(); i!=m_treeCache.end(); ++i )
            if ( (*i).second->lastUseMs < (*oldest).second->lastUseMs )
                oldest = i;
        m_treeCacheBytes -= (*oldest).second->bytes;
        GUCEF_DELETE (*oldest).second;
        m_treeCache.erase( oldest );
    }
    CCachedTree* entry = GUCEF_NEW CCachedTree();
    entry->tree = tree;
    entry->lastUseMs = nowMs;
    entry->bytes = bytes;
    TTreeCache::iterator existing = m_treeCache.find( key );
    if ( existing != m_treeCache.end() )
    {
        m_treeCacheBytes -= (*existing).second->bytes;
        GUCEF_DELETE (*existing).second;
    }
    m_treeCache[ key ] = entry;
    m_treeCacheBytes += bytes;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnTreeBuilt( CServeHandle& handle )
{GUCEF_TRACE;

    CTreeJob* job = static_cast< CTreeJob* >( handle.job.GetPointerAlways() );
    TDRNResourceStatus status = job->status;
    TTreePtr tree = job->tree;
    bool loaded = job->loaded;
    CORE::CString key = handle.treeKey;
    handle.job.Unlink();
    handle.future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    if ( m_treeBuilds > 0 )
        --m_treeBuilds;

    if ( DRN_RESSTATUS_OK == status )
    {
        if ( loaded )
            ++m_stats.treesLoaded;
        else
            ++m_stats.treesBuilt;
        CacheTree( key, tree, CDRNClock::NowInMilliseconds() );
    }

    // the handle that built it and every handle that waited for the same tree
    for ( UInt32 i=0; i<m_handles.size(); )
    {
        CServeHandle* waiting = m_handles[ i ];
        bool same = waiting == &handle || ( CServeHandle::STATE_AWAITING_TREE == waiting->state && waiting->treeKey == key );
        if ( !same )
        {
            ++i;
            continue;
        }
        for ( UInt32 q=0; q<m_treeQueue.size(); ++q )
        {
            if ( m_treeQueue[ q ] == waiting )
            {
                m_treeQueue.erase( m_treeQueue.begin() + q );
                break;
            }
        }
        if ( DRN_RESSTATUS_TOO_LARGE == status )
        {
            // no tree for it: served without, the requester decides whether that will do
            waiting->state = CServeHandle::STATE_OPEN;
            waiting->lastActivityMs = CDRNClock::NowInMilliseconds();
            SendInfo( waiting->peerId, waiting->handle, DRN_RESSTATUS_OK, CORE::CString::Empty, &waiting->info );
            ++i;
            continue;
        }
        if ( DRN_RESSTATUS_OK != status )
        {
            SendInfo( waiting->peerId, waiting->handle, status, DRN_RESSTATUS_CHANGED == status ? "the resource changed" : "the resource could not be read", GUCEF_NULL );
            DropHandle( waiting );
            continue;
        }
        waiting->tree = tree;
        waiting->info.hasTreeRoot = tree->GetRoot( waiting->info.treeRoot );
        waiting->state = CServeHandle::STATE_OPEN;
        waiting->lastActivityMs = CDRNClock::NowInMilliseconds();
        SendInfo( waiting->peerId, waiting->handle, DRN_RESSTATUS_OK, CORE::CString::Empty, &waiting->info );
        ++i;
    }
    StartTreeBuilds();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnRead( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handleId = 0;
    UInt32 readId = 0;
    UInt64 firstChunk = 0;
    UInt64 chunkCount = 0;
    UInt32 credit = 0;
    if ( !reader.ReadVarUInt32( handleId ) || !reader.ReadVarUInt32( readId ) || !reader.ReadVarUInt64( firstChunk ) ||
         !reader.ReadVarUInt64( chunkCount ) || !reader.ReadVarUInt32( credit ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    CServeHandle* handle = FindHandle( peerId, handleId );
    if ( GUCEF_NULL == handle || CServeHandle::STATE_OPEN != handle->state )
    {
        SendReadEnd( peerId, handleId, readId, DRN_RESSTATUS_CLOSED, "the resource is not open" );
        return;
    }
    UInt64 chunks = handle->info.GetChunkCount();
    if ( firstChunk > chunks || chunkCount > chunks - firstChunk || 0 == chunkCount )
    {
        SendReadEnd( peerId, handleId, readId, DRN_RESSTATUS_OUT_OF_RANGE, "the read reaches beyond the end of the resource" );
        return;
    }
    if ( GUCEF_NULL != handle->FindRead( readId ) )
    {
        SendReadEnd( peerId, handleId, readId, DRN_RESSTATUS_PROTOCOL_ERROR, "the read id is in use" );
        return;
    }
    CORE::CString busyWhy;
    CDRNPeerLimits peerLimits = GetPeerLimits( peerId );
    if ( IsLimitReached( 0 != m_settings.maxActiveReads ? CountAllReads() : 0, m_settings.maxActiveReads, 0 != peerLimits.maxActiveReads ? CountReads( peerId ) : 0,
                         peerLimits.maxActiveReads, "reads under way", busyWhy ) )
    {
        SendReadEnd( peerId, handleId, readId, DRN_RESSTATUS_BUSY, busyWhy );
        return;
    }
    handle->lastActivityMs = CDRNClock::NowInMilliseconds();
    CServeRead* read = GUCEF_NEW CServeRead();
    read->readId = readId;
    read->nextToRead = firstChunk;
    read->end = firstChunk + chunkCount;
    read->credit = credit;
    handle->reads.push_back( read );
    IssueReads( *handle );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnCredit( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handleId = 0;
    UInt32 readId = 0;
    UInt32 credit = 0;
    if ( !reader.ReadVarUInt32( handleId ) || !reader.ReadVarUInt32( readId ) || !reader.ReadVarUInt32( credit ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    CServeHandle* handle = FindHandle( peerId, handleId );
    CServeRead* read = GUCEF_NULL != handle ? handle->FindRead( readId ) : GUCEF_NULL;
    if ( GUCEF_NULL == read )
        return;     // a read that ended meanwhile
    handle->lastActivityMs = CDRNClock::NowInMilliseconds();
    read->credit += credit;
    IssueReads( *handle );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnCancel( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handleId = 0;
    UInt32 readId = 0;
    if ( !reader.ReadVarUInt32( handleId ) || !reader.ReadVarUInt32( readId ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    CServeHandle* handle = FindHandle( peerId, handleId );
    if ( GUCEF_NULL == handle )
        return;
    for ( UInt32 i=0; i<handle->reads.size(); ++i )
    {
        CServeRead* read = handle->reads[ i ];
        if ( read->readId == readId )
        {
            UpdateServerBuffered( -(Int64) read->GetHeldBytes() );
            handle->reads.erase( handle->reads.begin() + i );
            GUCEF_DELETE read;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnClose( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handleId = 0;
    if ( !reader.ReadVarUInt32( handleId ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    CServeHandle* handle = FindHandle( peerId, handleId );
    if ( GUCEF_NULL != handle )
    {
        DropHandle( handle );
        StartTreeBuilds();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::IssueReads( CServeHandle& handle )
{GUCEF_TRACE;

    if ( CServeHandle::STATE_OPEN != handle.state )
        return;
    for ( UInt32 r=0; r<handle.reads.size(); ++r )
    {
        CServeRead& read = *handle.reads[ r ];
        if ( !read.job.IsNULL() || read.nextToRead >= read.end )
            continue;
        // read ahead again once half of what was read went out: runs of at least half the read ahead, never more
        // than the credit allows
        UInt64 ahead = read.GetReadyChunks();
        if ( ahead >= read.credit || ahead > m_settings.readAheadChunks / 2 )
            continue;
        UInt64 count = read.credit - ahead;
        if ( count > m_settings.readAheadChunks - ahead )
            count = m_settings.readAheadChunks - ahead;
        if ( count > read.end - read.nextToRead )
            count = read.end - read.nextToRead;

        // within the buffer of the serving side and, when the member has a buffer limit of its own, within that: whichever
        // is smaller. One chunk always, so that nothing waits forever: when the server's buffer is empty and the member's
        // own holds nothing
        UInt64 room = m_stats.serverBufferedBytes < m_settings.maxBufferedBytes ? m_settings.maxBufferedBytes - m_stats.serverBufferedBytes : 0;
        UInt64 fits = room / DRN_RESOURCE_CHUNK_SIZE;
        UInt64 peerHeld = 0;
        UInt64 peerFits = count;
        UInt32 peerBufferLimit = GetPeerLimits( handle.peerId ).maxBufferedBytes;
        if ( 0 != peerBufferLimit )
        {
            peerHeld = CountBufferedBytes( handle.peerId );
            peerFits = peerHeld < peerBufferLimit ? ( peerBufferLimit - peerHeld ) / DRN_RESOURCE_CHUNK_SIZE : 0;
        }
        if ( fits < count )
            count = fits;
        if ( peerFits < count )
            count = peerFits;
        if ( 0 == count )
        {
            if ( ( 0 == fits && 0 != m_stats.serverBufferedBytes ) || ( 0 == peerFits && 0 != peerHeld ) )
                continue;
            count = 1;
        }

        CReadJob* job = GUCEF_NEW CReadJob();
        job->provider = handle.provider;
        job->name = handle.name;
        job->info = handle.info;
        job->firstChunk = read.nextToRead;
        job->chunkCount = (UInt32) count;
        read.job = CIDRNResourceIoJobPtr( job );
        read.future = CDRNResourceIoTaskConsumer::Queue( m_settings.threadPoolName, read.job, m_session->GetNode().GetWorkPulseGenerator() );
        if ( read.future.HasNoFuture() )
        {
            read.job.Unlink();
            continue;
        }
        read.jobFirst = read.nextToRead;
        read.jobChunks = (UInt32) count;
        read.jobReservedBytes = (UInt32) ( count * DRN_RESOURCE_CHUNK_SIZE );
        read.nextToRead += count;
        UpdateServerBuffered( (Int64) read.jobReservedBytes );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::CollectRead( CServeHandle& handle, CServeRead& read )
{GUCEF_TRACE;

    CReadJob* job = static_cast< CReadJob* >( read.job.GetPointerAlways() );
    TDRNResourceStatus status = job->status;
    CReadRun* run = GUCEF_NEW CReadRun();
    run->first = read.jobFirst;
    run->count = read.jobChunks;
    run->reservedBytes = read.jobReservedBytes;
    run->job = read.job;
    run->data = &job->data;
    read.job.Unlink();
    read.future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    read.jobReservedBytes = 0;

    if ( DRN_RESSTATUS_OK != status )
    {
        UpdateServerBuffered( -(Int64) run->reservedBytes );
        GUCEF_DELETE run;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceSessionComponent: A read of \"" << handle.providerName << "/" << handle.name << "\" for member " <<
                   handle.peerId << " failed: " << CDRNResourceTransferResult::StatusToString( status ) );
        SendReadEnd( handle.peerId, handle.handle, read.readId, status, DRN_RESSTATUS_CHANGED == status ? "the resource changed" : "the resource could not be read" );
        for ( UInt32 i=0; i<handle.reads.size(); ++i )
        {
            if ( handle.reads[ i ] == &read )
            {
                UpdateServerBuffered( -(Int64) read.GetHeldBytes() );
                handle.reads.erase( handle.reads.begin() + i );
                GUCEF_DELETE &read;
                break;
            }
        }
        return;
    }
    read.runs.push_back( run );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::TickServing( UInt64 nowMs )
{GUCEF_TRACE;

    for ( UInt32 h=0; h<m_handles.size(); )
    {
        CServeHandle* handle = m_handles[ h ];
        if ( !handle->job.IsNULL() && handle->future.IsReady() )
        {
            UInt32 before = (UInt32) m_handles.size();
            if ( CServeHandle::STATE_RESOLVING == handle->state )
                OnResolved( *handle );
            else
            if ( CServeHandle::STATE_BUILDING_TREE == handle->state )
                OnTreeBuilt( *handle );
            if ( m_handles.size() != before )
            {
                h = 0;      // handles went: start over, those already done have nothing due
                continue;
            }
        }
        if ( CServeHandle::STATE_OPEN == handle->state && handle->reads.empty() && 0 != m_settings.handleIdleTimeoutMs &&
             nowMs >= handle->lastActivityMs + m_settings.handleIdleTimeoutMs )
        {
            // idle too long: closed; the requester's next read is answered CLOSED and it opens the resource again
            ++m_stats.handlesExpired;
            DropHandle( handle );
            continue;
        }
        if ( CServeHandle::STATE_OPEN == handle->state )
        {
            for ( UInt32 r=0; r<handle->reads.size(); )
            {
                CServeRead* read = handle->reads[ r ];
                if ( !read->job.IsNULL() && read->future.IsReady() )
                {
                    UInt32 before = (UInt32) handle->reads.size();
                    CollectRead( *handle, *read );
                    if ( handle->reads.size() != before )
                        continue;
                }
                ++r;
            }
            IssueReads( *handle );
        }
        ++h;
    }
    SendChunks( nowMs );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::UpdatePeerUploadLimiter( CPeerBudget& budget, UInt32 bytesPerSecond )
{GUCEF_TRACE;

    if ( 0 != bytesPerSecond && budget.uploadLimiterRate != bytesPerSecond )
    {
        GUCEF_DELETE budget.uploadLimiter;
        UInt32 burst = bytesPerSecond / 4 > 4 * DRN_RESOURCE_CHUNK_SIZE ? bytesPerSecond / 4 : 4 * DRN_RESOURCE_CHUNK_SIZE;
        budget.uploadLimiter = GUCEF_NEW CORE::CRateLimiter( (Float64) bytesPerSecond, burst );
        budget.uploadLimiterRate = bytesPerSecond;
    }
    else if ( 0 == bytesPerSecond && GUCEF_NULL != budget.uploadLimiter )
    {
        GUCEF_DELETE budget.uploadLimiter;
        budget.uploadLimiter = GUCEF_NULL;
        budget.uploadLimiterRate = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::SendChunks( UInt64 /* nowMs */ )
{GUCEF_TRACE;

    if ( m_handles.empty() )
        return;

    // the upload limit of the settings, if any
    if ( 0 != m_settings.maxUploadBytesPerSecond && m_uploadLimiterRate != m_settings.maxUploadBytesPerSecond )
    {
        GUCEF_DELETE m_uploadLimiter;
        UInt32 burst = m_settings.maxUploadBytesPerSecond / 4 > 4 * DRN_RESOURCE_CHUNK_SIZE ? m_settings.maxUploadBytesPerSecond / 4 : 4 * DRN_RESOURCE_CHUNK_SIZE;
        m_uploadLimiter = GUCEF_NEW CORE::CRateLimiter( (Float64) m_settings.maxUploadBytesPerSecond, burst );
        m_uploadLimiterRate = m_settings.maxUploadBytesPerSecond;
    }
    else if ( 0 == m_settings.maxUploadBytesPerSecond && GUCEF_NULL != m_uploadLimiter )
    {
        GUCEF_DELETE m_uploadLimiter;
        m_uploadLimiter = GUCEF_NULL;
        m_uploadLimiterRate = 0;
    }

    // the members that are served
    GUCEF::set< UInt32 > peers;
    for ( UInt32 h=0; h<m_handles.size(); ++h )
        if ( !m_handles[ h ]->reads.empty() )
            peers.insert( m_handles[ h ]->peerId );

    UInt8 header[ 64 + CDRNResourceHashTree::MaxAuditPathLength * CDRNResourceHashTree::HashSize ];
    GUCEF::vector< CServeHandle* > readHandles;
    GUCEF::vector< CServeRead* > reads;
    for ( GUCEF::set< UInt32 >::iterator p=peers.begin(); p!=peers.end(); ++p )
    {
        UInt32 peerId = (*p);
        CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
        if ( link.IsNULL() || !link->IsOperational() )
            continue;
        bool datagram = !link->GetChannel().IsNULL() && link->GetChannel()->GetCapabilities().isDatagram;

        reads.clear();
        readHandles.clear();
        for ( UInt32 h=0; h<m_handles.size(); ++h )
        {
            CServeHandle* handle = m_handles[ h ];
            if ( handle->peerId != peerId || CServeHandle::STATE_OPEN != handle->state )
                continue;
            for ( UInt32 r=0; r<handle->reads.size(); ++r )
            {
                readHandles.push_back( handle );
                reads.push_back( handle->reads[ r ] );
            }
        }
        if ( reads.empty() )
            continue;

        CPeerBudget& budget = GetBudget( peerId );
        UpdatePeerUploadLimiter( budget, GetPeerLimits( peerId ).maxUploadBytesPerSecond );
        UInt32 turn = budget.serveTurn % (UInt32) reads.size();
        bool progress = true;
        bool blocked = false;
        while ( progress && !blocked )
        {
            progress = false;
            for ( UInt32 n=0; n<reads.size() && !blocked; ++n )
            {
                if ( link->GetSendBacklogBytes() >= m_settings.maxLinkBacklogBytes ||
                     ( datagram && link->GetUnackedCount() >= m_settings.maxUnackedMessages ) )
                {
                    blocked = true;
                    break;
                }
                CServeHandle& handle = *readHandles[ ( turn + n ) % reads.size() ];
                CServeRead& read = *reads[ ( turn + n ) % reads.size() ];
                if ( 0 == read.credit || read.runs.empty() )
                    continue;
                CReadRun& run = *read.runs.front();
                UInt64 chunk = run.first + run.sent;
                UInt32 offset = (UInt32) ( run.sent * DRN_RESOURCE_CHUNK_SIZE );
                UInt32 size = handle.info.GetChunkBytes( chunk );
                if ( offset + size > run.data->GetDataSize() )
                    size = run.data->GetDataSize() > offset ? run.data->GetDataSize() - offset : 0;
                // the member's own upload rate and the server's, both: the chunk goes when both have room for it
                if ( ( GUCEF_NULL != budget.uploadLimiter && budget.uploadLimiter->GetAvailableTokens() < (Float64) size ) ||
                     ( GUCEF_NULL != m_uploadLimiter && m_uploadLimiter->GetAvailableTokens() < (Float64) size ) )
                {
                    blocked = true;
                    break;
                }
                if ( GUCEF_NULL != budget.uploadLimiter )
                    budget.uploadLimiter->TryAcquire( size );
                if ( GUCEF_NULL != m_uploadLimiter )
                    m_uploadLimiter->TryAcquire( size );

                UInt8 path[ CDRNResourceHashTree::MaxAuditPathLength * CDRNResourceHashTree::HashSize ];
                UInt32 pathLength = handle.tree.IsNULL() ? 0 : handle.tree->GetAuditPath( chunk, path );
                UInt32 headerSize = CDRNWireWriter::EncodeVarUInt64( handle.handle, header );
                headerSize += CDRNWireWriter::EncodeVarUInt64( read.readId, header + headerSize );
                headerSize += CDRNWireWriter::EncodeVarUInt64( chunk, header + headerSize );
                header[ headerSize++ ] = (UInt8) pathLength;
                if ( pathLength > 0 )
                {
                    memcpy( header + headerSize, path, pathLength * CDRNResourceHashTree::HashSize );
                    headerSize += pathLength * CDRNResourceHashTree::HashSize;
                }
                const UInt8* bytes = static_cast< const UInt8* >( run.data->GetConstBufferPtr() ) + offset;
                if ( !link->SendGather( DRN_LANE_RELIABLE_UNORDERED, DRN_MSGCLASS_RESOURCE, DRN_RES_DATA, header, headerSize, bytes, size ) )
                {
                    blocked = true;
                    break;
                }
                progress = true;
                ++run.sent;
                --read.credit;
                ++m_stats.chunksServed;
                m_stats.bytesServed += size;
                UInt32 released = run.sent >= run.count || run.reservedBytes < DRN_RESOURCE_CHUNK_SIZE ? run.reservedBytes : DRN_RESOURCE_CHUNK_SIZE;
                run.reservedBytes -= released;
                UpdateServerBuffered( -(Int64) released );
                if ( run.sent >= run.count )
                {
                    GUCEF_DELETE read.runs.front();
                    read.runs.erase( read.runs.begin() );
                }
            }
            ++turn;
        }
        budget.serveTurn = turn;

        // reads that sent everything are done: the requester counts its chunks
        for ( UInt32 h=0; h<m_handles.size(); ++h )
        {
            CServeHandle& handle = *m_handles[ h ];
            if ( handle.peerId != peerId )
                continue;
            for ( UInt32 r=0; r<handle.reads.size(); )
            {
                CServeRead* read = handle.reads[ r ];
                if ( read->nextToRead >= read->end && read->job.IsNULL() && read->runs.empty() )
                {
                    ++m_stats.readsServed;
                    handle.reads.erase( handle.reads.begin() + r );
                    GUCEF_DELETE read;
                    continue;
                }
                ++r;
            }
            IssueReads( handle );
        }
    }
}

/*-------------------------------------------------------------------------*/
//  Requesting
/*-------------------------------------------------------------------------*/

CDRNResourceTransfer*
CDRNResourceSessionComponent::FindTransfer( UInt32 peerId, UInt32 handle ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        if ( m_transfers[ i ]->m_handle == handle && m_transfers[ i ]->m_peerId == peerId )
            return m_transfers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnInfo( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handle = 0;
    if ( !reader.ReadVarUInt32( handle ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    CDRNResourceTransfer* transfer = FindTransfer( peerId, handle );
    if ( GUCEF_NULL != transfer )
    {
        transfer->OnInfo( reader );
        return;
    }
    CDRNResourceHandle* clientHandle = FindClientHandle( peerId, handle );
    if ( GUCEF_NULL == clientHandle )
    {
        ++m_stats.droppedMessages;
        return;
    }

    UInt32 statusValue = 0;
    CORE::CString message;
    CDRNResourceInfo info;
    if ( !reader.ReadVarUInt32( statusValue ) || !reader.ReadString( message ) )
    {
        clientHandle->OnInfo( DRN_RESSTATUS_PROTOCOL_ERROR, "a malformed INFO", info );
        return;
    }
    TDRNResourceStatus status = CDRNResourceTransferResult::StatusFromWire( statusValue );
    if ( DRN_RESSTATUS_OK == status )
    {
        UInt32 chunkSize = 0;
        UInt8 flags = 0;
        if ( !reader.ReadVarUInt64( info.size ) || !reader.ReadVarUInt64( info.modifiedUtcMs ) || !reader.ReadString( info.revision ) ||
             !reader.ReadVarUInt32( chunkSize ) || !reader.ReadUInt8( flags ) )
        {
            status = DRN_RESSTATUS_PROTOCOL_ERROR;
            message = "a malformed INFO";
        }
        else
        {
            info.hasSha256 = 0 != ( flags & DRN_RES_DIGEST_SHA256 );
            info.hasTreeRoot = 0 != ( flags & DRN_RES_DIGEST_TREE );
            if ( ( info.hasSha256 && !reader.ReadBytes( info.sha256, CDRNResourceInfo::DigestSize ) ) ||
                 ( info.hasTreeRoot && !reader.ReadBytes( info.treeRoot, CDRNResourceInfo::DigestSize ) ) || DRN_RESOURCE_CHUNK_SIZE != chunkSize )
            {
                status = DRN_RESSTATUS_PROTOCOL_ERROR;
                message = "a malformed INFO or another chunk size";
            }
        }
    }
    clientHandle->OnInfo( status, message, info );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnData( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handle = 0;
    UInt32 readId = 0;
    UInt64 chunk = 0;
    UInt8 pathLength = 0;
    if ( !reader.ReadVarUInt32( handle ) || !reader.ReadVarUInt32( readId ) || !reader.ReadVarUInt64( chunk ) || !reader.ReadUInt8( pathLength ) ||
         pathLength > CDRNResourceHashTree::MaxAuditPathLength || reader.GetRemaining() < (UInt32) pathLength * CDRNResourceHashTree::HashSize )
    {
        ++m_stats.droppedMessages;
        return;
    }
    const UInt8* path = reader.GetCurrentPtr();
    reader.SkipBytes( (UInt32) pathLength * CDRNResourceHashTree::HashSize );
    const UInt8* data = reader.GetCurrentPtr();
    UInt32 dataSize = reader.GetRemaining();

    CDRNResourceTransfer* transfer = FindTransfer( peerId, handle );
    if ( GUCEF_NULL != transfer )
    {
        transfer->OnData( readId, chunk, path, pathLength, data, dataSize );
        return;
    }
    CDRNResourceHandle* clientHandle = FindClientHandle( peerId, handle );
    if ( GUCEF_NULL != clientHandle )
        clientHandle->OnData( readId, chunk, path, pathLength, data, dataSize );
    else
        ++m_stats.droppedMessages;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnReadEnd( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 handle = 0;
    UInt32 readId = 0;
    UInt32 status = 0;
    CORE::CString message;
    if ( !reader.ReadVarUInt32( handle ) || !reader.ReadVarUInt32( readId ) || !reader.ReadVarUInt32( status ) || !reader.ReadString( message ) )
    {
        ++m_stats.droppedMessages;
        return;
    }
    CDRNResourceTransfer* transfer = FindTransfer( peerId, handle );
    if ( GUCEF_NULL != transfer )
    {
        transfer->OnReadEnd( readId, CDRNResourceTransferResult::StatusFromWire( status ), message );
        return;
    }
    CDRNResourceHandle* clientHandle = FindClientHandle( peerId, handle );
    if ( GUCEF_NULL != clientHandle )
        clientHandle->OnReadEnd( readId, CDRNResourceTransferResult::StatusFromWire( status ), message );
}

/*-------------------------------------------------------------------------*/

CDRNResourceSessionComponent::CPeerBudget&
CDRNResourceSessionComponent::GetBudget( UInt32 peerId )
{GUCEF_TRACE;

    TPeerBudgetMap::iterator i = m_budgets.find( peerId );
    if ( i != m_budgets.end() )
        return *(*i).second;
    CPeerBudget* budget = GUCEF_NEW CPeerBudget();
    m_budgets[ peerId ] = budget;
    return *budget;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::AddLinkOutstanding( UInt32 peerId, Int64 delta )
{GUCEF_TRACE;

    if ( 0 == delta )
        return;
    CPeerBudget& budget = GetBudget( peerId );
    budget.outstanding += delta;
    if ( budget.outstanding < 0 )
        budget.outstanding = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::UpdateClientBuffered( Int64 delta )
{GUCEF_TRACE;

    Int64 value = (Int64) m_stats.clientBufferedBytes + delta;
    m_stats.clientBufferedBytes = value < 0 ? 0 : (UInt64) value;
    if ( m_stats.clientBufferedBytes > m_stats.peakClientBufferedBytes )
        m_stats.peakClientBufferedBytes = m_stats.clientBufferedBytes;
}

/*-------------------------------------------------------------------------*/

CDRNResourceSessionComponent::CPeerBudget&
CDRNResourceSessionComponent::UpdateBudget( UInt32 peerId, const CDRNPeerLink& link )
{GUCEF_TRACE;

    // the rate: below this node's own receive limits, so that what is asked for never trips them
    const CDRNPolicyLimits& limits = m_session->GetNode().GetSettings().securityPolicy.limits;
    bool datagram = !link.GetChannel().IsNULL() && link.GetChannel()->GetCapabilities().isDatagram;
    UInt64 rate = 0;
    if ( 0 != limits.maxBytesPerSecond )
        rate = (UInt64) limits.maxBytesPerSecond * m_settings.receiveLimitSharePercent / 100;
    if ( datagram && 0 != limits.maxMessagesPerSecond )
    {
        UInt64 messageRate = (UInt64) limits.maxMessagesPerSecond * m_settings.receiveLimitSharePercent / 100 * link.GetMaxUnfragmentedPayload();
        if ( 0 == rate || messageRate < rate )
            rate = messageRate;
    }
    if ( 0 != m_settings.maxDownloadBytesPerSecond && ( 0 == rate || m_settings.maxDownloadBytesPerSecond < rate ) )
        rate = m_settings.maxDownloadBytesPerSecond;
    if ( rate > 0xFFFFFFFF )
        rate = 0xFFFFFFFF;

    CPeerBudget& budget = GetBudget( peerId );
    if ( (UInt32) rate != budget.limiterRate )
    {
        GUCEF_DELETE budget.limiter;
        budget.limiter = GUCEF_NULL;
        budget.limiterRate = (UInt32) rate;
        if ( 0 != rate )
        {
            UInt64 burst = rate / 4 > 2 * DRN_RESOURCE_CHUNK_SIZE ? rate / 4 : 2 * DRN_RESOURCE_CHUNK_SIZE;
            budget.limiterBurst = (UInt32) burst;
            budget.limiter = GUCEF_NEW CORE::CRateLimiter( (Float64) rate, (UInt32) burst );
        }
    }
    return budget;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::AcquireReceiveBudget( UInt32 peerId, UInt32 bytes )
{GUCEF_TRACE;

    CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
    if ( link.IsNULL() )
        return 0;
    CPeerBudget& budget = UpdateBudget( peerId, *link );
    if ( GUCEF_NULL == budget.limiter )
        return 0;
    if ( bytes > budget.limiterBurst )
        bytes = budget.limiterBurst;
    if ( budget.limiter->TryAcquire( bytes ) )
        return 0;
    UInt32 delay = budget.limiter->GetDelayUntilAvailableInMs( bytes );
    return 0 == delay ? 1 : ( delay > 1000 ? 1000 : delay );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::ShareCredit( UInt64 /* nowMs */ )
{GUCEF_TRACE;

    if ( m_transfers.empty() )
        return;

    GUCEF::set< UInt32 > peers;
    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        if ( CDRNResourceTransfer::TRANSFERSTATE_TRANSFERRING == m_transfers[ i ]->m_state )
            peers.insert( m_transfers[ i ]->m_peerId );

    const CDRNPolicyLimits& limits = m_session->GetNode().GetSettings().securityPolicy.limits;
    GUCEF::vector< CDRNResourceTransfer* > transfers;
    for ( GUCEF::set< UInt32 >::iterator p=peers.begin(); p!=peers.end(); ++p )
    {
        UInt32 peerId = (*p);
        CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
        if ( link.IsNULL() || !link->IsOperational() )
            continue;
        bool datagram = !link->GetChannel().IsNULL() && link->GetChannel()->GetCapabilities().isDatagram;

        // the cap of credit on the link: the settings, and on datagram links half the fragment groups this node takes
        Int64 cap = m_settings.maxCreditChunksPerLink;
        if ( datagram && 0 != limits.maxFragmentGroups && (Int64) ( limits.maxFragmentGroups / 2 ) < cap )
            cap = limits.maxFragmentGroups / 2;
        if ( cap < 1 )
            cap = 1;

        CPeerBudget& budget = UpdateBudget( peerId, *link );

        transfers.clear();
        for ( UInt32 i=0; i<m_transfers.size(); ++i )
            if ( m_transfers[ i ]->m_peerId == peerId && CDRNResourceTransfer::TRANSFERSTATE_TRANSFERRING == m_transfers[ i ]->m_state )
                transfers.push_back( m_transfers[ i ] );

        // one chunk per transfer per round, from a turn that moves on every tick
        UInt32 start = m_creditRound % (UInt32) transfers.size();
        bool progress = true;
        while ( progress && budget.outstanding < cap )
        {
            progress = false;
            for ( UInt32 n=0; n<transfers.size() && budget.outstanding<cap; ++n )
            {
                CDRNResourceTransfer* transfer = transfers[ ( start + n ) % transfers.size() ];
                if ( 0 == transfer->GetWantedCredit() )
                    continue;
                if ( GUCEF_NULL != budget.limiter && !budget.limiter->TryAcquire( DRN_RESOURCE_CHUNK_SIZE ) )
                {
                    progress = false;
                    n = (UInt32) transfers.size();
                    break;
                }
                transfer->AddPendingCredit( 1 );
                ++budget.outstanding;
                progress = true;
            }
        }
        // the pending credit was counted above already: FlushCredit() adds it again
        for ( UInt32 n=0; n<transfers.size(); ++n )
        {
            budget.outstanding -= transfers[ n ]->m_pendingCredit;
            transfers[ n ]->FlushCredit();
        }
    }
    ++m_creditRound;
}

/*-------------------------------------------------------------------------*/
//  Namespace: serving
/*-------------------------------------------------------------------------*/

/** The bytes of a string without its terminator */
static UInt32
NsTextBytes( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** What the shortest answer a page may be cut to leaves for the header of the answer and of the page */
static const UInt32 NsAnswerHeaderReserve = 1024;

/** The most bytes a STAT answer takes per path */
static const UInt32 NsStatItemBytes = 340;

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNsAnswerBytes( void ) const
{GUCEF_TRACE;

    UInt32 bytes = 0 != m_settings.maxNamespaceAnswerBytes ? m_settings.maxNamespaceAnswerBytes : 512 * 1024;
    UInt32 policyMax = m_session->GetNode().GetSettings().securityPolicy.limits.maxMessageSize;
    if ( 0 != policyMax && bytes > policyMax )
        bytes = policyMax;
    if ( bytes < 4 * NsAnswerHeaderReserve )
        bytes = 4 * NsAnswerHeaderReserve;
    return bytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNsStatLimit( const CIDRNResourceProvider& provider ) const
{GUCEF_TRACE;

    UInt32 limit = 0 != m_settings.maxNamespaceStatPaths ? m_settings.maxNamespaceStatPaths : 256;
    UInt32 shareStat = 0;
    UInt32 sharePage = 0;
    provider.GetNamespaceLimits( shareStat, sharePage );
    if ( 0 != shareStat && shareStat < limit )
        limit = shareStat;
    UInt32 fits = ( GetNsAnswerBytes() - NsAnswerHeaderReserve ) / NsStatItemBytes;
    return limit < fits ? limit : fits;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNsPageLimit( const CIDRNResourceProvider& provider ) const
{GUCEF_TRACE;

    UInt32 limit = 0 != m_settings.maxNamespacePageEntries ? m_settings.maxNamespacePageEntries : 1000;
    UInt32 shareStat = 0;
    UInt32 sharePage = 0;
    provider.GetNamespaceLimits( shareStat, sharePage );
    if ( 0 != sharePage && sharePage < limit )
        limit = sharePage;
    return limit;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::CountNsServing( UInt32 peerId ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_nsServing.size(); ++i )
        if ( m_nsServing[ i ]->peerId == peerId )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNamespaceListingCount( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_nsListings.size(); ++i )
        if ( 0 == peerId || m_nsListings[ i ]->peerId == peerId )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CDRNResourceSessionComponent::CNsListing*
CDRNResourceSessionComponent::FindNsListing( UInt32 peerId, UInt32 listingId ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_nsListings.size(); ++i )
        if ( m_nsListings[ i ]->peerId == peerId && m_nsListings[ i ]->id == listingId )
            return m_nsListings[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropNsServe( CNsServe* serve )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_nsServing.size(); ++i )
    {
        if ( m_nsServing[ i ] == serve )
        {
            m_nsServing.erase( m_nsServing.begin() + i );
            break;
        }
    }
    GUCEF_DELETE serve;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropNsListing( CNsListing* listing )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_nsListings.size(); ++i )
    {
        if ( m_nsListings[ i ] == listing )
        {
            m_nsListings.erase( m_nsListings.begin() + i );
            break;
        }
    }
    GUCEF_DELETE listing;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropNamespaceOf( UInt32 peerId, bool allPeers )
{GUCEF_TRACE;

    // the requests under way first: they point at listings
    for ( UInt32 i=0; i<m_nsServing.size(); )
    {
        if ( allPeers || m_nsServing[ i ]->peerId == peerId )
        {
            GUCEF_DELETE m_nsServing[ i ];
            m_nsServing.erase( m_nsServing.begin() + i );
            continue;
        }
        ++i;
    }
    for ( UInt32 i=0; i<m_nsListings.size(); )
    {
        if ( allPeers || m_nsListings[ i ]->peerId == peerId )
        {
            ++m_stats.namespaceListingsExpired;
            GUCEF_DELETE m_nsListings[ i ];
            m_nsListings.erase( m_nsListings.begin() + i );
            continue;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::DropNamespaceOfProvider( const CORE::CString& providerName )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_nsServing.size(); )
    {
        CNsServe* serve = m_nsServing[ i ];
        if ( serve->share == providerName )
        {
            SendNsAnswer( serve->peerId, serve->requestId, serve->kind, DRN_RESSTATUS_CLOSED, "the provider is no longer served" );
            GUCEF_DELETE serve;
            m_nsServing.erase( m_nsServing.begin() + i );
            continue;
        }
        ++i;
    }
    for ( UInt32 i=0; i<m_nsListings.size(); )
    {
        if ( m_nsListings[ i ]->share == providerName )
        {
            GUCEF_DELETE m_nsListings[ i ];
            m_nsListings.erase( m_nsListings.begin() + i );
            continue;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceSessionComponent::IsHiddenByPolicy( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName,
                                                const CORE::CString& name, bool leafIsDirectory )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;
    CIDRNAccessPolicy& policy = m_session->GetNode().GetAccessPolicy();

    // a directory that is hidden hides everything in it: every directory above the name is asked as well
    UInt32 size = NsTextBytes( name );
    const char* text = name.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '/' == text[ i ] )
        {
            CORE::CString parent;
            parent.Set( text, i );
            if ( !policy.MayListEntryAs( peerId, auth, providerName, parent, true ) )
                return true;
        }
    }
    return !policy.MayListEntryAs( peerId, auth, providerName, name, leafIsDirectory );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::SendNsAnswer( UInt32 peerId, UInt32 requestId, UInt32 kind, TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    CDRNNamespaceCodec::WriteAnswerHeader( writer, requestId, kind, status, message );
    Send( peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_NS_ANSWER, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::RefuseNs( UInt32 peerId, UInt32 requestId, UInt32 kind, TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    ++m_stats.namespaceRequestsRefused;
    SendNsAnswer( peerId, requestId, kind, status, message );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnNamespaceMessage( UInt32 peerId, CDRNPeerLink& link, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    UInt32 opcode = message.opcode;
    if ( DRN_RES_NS_ANSWER == opcode )
    {
        CDRNWireReader reader( message.payload );
        OnNsAnswer( peerId, reader );
        return;
    }

    // a request: without a request id there is nothing to answer
    UInt32 requestId = 0;
    if ( !CDRNNamespaceCodec::PeekRequestId( message.payload.GetConstBufferPtr(), message.payload.GetDataSize(), requestId ) )
    {
        ++m_stats.droppedMessages;
        ++m_stats.namespaceMalformed;
        return;
    }
    bool isMutation = opcode >= DRN_RES_NS_MUTATION_FIRST && opcode <= DRN_RES_NS_MUTATION_LAST;
    bool isRead = DRN_RES_NS_HELLO == opcode || DRN_RES_NS_SHARES == opcode || DRN_RES_NS_STAT == opcode || DRN_RES_NS_LIST == opcode;
    if ( !isMutation && !isRead && !( opcode >= DRN_RES_NS_HELLO && opcode < DRN_RES_NS_ANSWER ) && !( opcode >= DRN_RES_NS_RESERVED_FIRST && opcode <= DRN_RES_NS_RESERVED_LAST ) )
    {
        ++m_stats.droppedMessages;      // the numbers of answers that no side sends to a serving side
        return;
    }

    // in this order, so that a refusal tells nothing about what exists
    if ( !link.IsProtected() && !m_settings.allowUnprotectedLinks )
    {
        RefuseNs( peerId, requestId, opcode, DRN_RESSTATUS_INSECURE_LINK, "resources are served over protected links only" );
        return;
    }
    if ( m_session->GetNode().GetSettings().protocolMinor < DRN_PROTOCOL_MINOR_NAMESPACE )
    {
        RefuseNs( peerId, requestId, opcode, DRN_RESSTATUS_UNSUPPORTED, "this node has no namespace operations (protocol below 2.3)" );
        return;
    }
    if ( isMutation )
    {
        RefuseNs( peerId, requestId, opcode, DRN_RESSTATUS_READ_ONLY, "shares are read-only" );
        return;
    }
    if ( !isRead )
    {
        RefuseNs( peerId, requestId, opcode, DRN_RESSTATUS_UNSUPPORTED, "no such namespace operation in this version" );
        return;
    }
    CDRNWireReader reader( message.payload );
    switch ( opcode )
    {
        case DRN_RES_NS_HELLO:      { OnNsHello( peerId, requestId, reader ); break; }
        case DRN_RES_NS_SHARES:     { OnNsShares( peerId, requestId ); break; }
        case DRN_RES_NS_STAT:       { OnNsStat( peerId, requestId, reader ); break; }
        case DRN_RES_NS_LIST:       { OnNsList( peerId, requestId, reader ); break; }
        default:                    { break; }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnNsHello( UInt32 peerId, UInt32 requestId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 id = 0;
    UInt32 maxVersion = 0;
    UInt32 clientCaps = 0;
    if ( !CDRNNamespaceCodec::ReadHelloRequest( reader, id, maxVersion, clientCaps ) )
    {
        ++m_stats.namespaceMalformed;
        RefuseNs( peerId, requestId, DRN_RES_NS_HELLO, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed request" );
        return;
    }
    if ( maxVersion < DRN_NS_VERSION )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_HELLO, DRN_RESSTATUS_UNSUPPORTED, "no namespace protocol version in common" );
        return;
    }
    CDRNNamespaceInfo info;
    info.version = maxVersion < DRN_NS_VERSION ? maxVersion : DRN_NS_VERSION;
    info.capabilities = DRN_NS_CAP_READ | DRN_NS_CAP_LIST | DRN_NS_CAP_STAT;
    info.maxStatPaths = 0 != m_settings.maxNamespaceStatPaths ? m_settings.maxNamespaceStatPaths : 256;
    UInt32 fits = ( GetNsAnswerBytes() - NsAnswerHeaderReserve ) / NsStatItemBytes;
    if ( info.maxStatPaths > fits )
        info.maxStatPaths = fits;
    info.maxPageEntries = 0 != m_settings.maxNamespacePageEntries ? m_settings.maxNamespacePageEntries : 1000;
    info.maxAnswerBytes = GetNsAnswerBytes();
    info.listingLifetimeMs = m_settings.namespaceListingLifetimeMs;

    // the caps that apply to the member that asks: the lower of the server-wide limit and the member's own (what the policy
    // made of it), 0 = no maximum. Not what is free at the moment
    if ( m_settings.announceLimits )
    {
        CDRNPeerLimits peerLimits = GetPeerLimits( peerId );
        info.limitsAnnounced = true;
        info.maxOpenHandles = LowerLimit( m_settings.maxOpenHandles, peerLimits.maxOpenHandles );
        info.maxActiveReads = LowerLimit( m_settings.maxActiveReads, peerLimits.maxActiveReads );
        info.maxNamespaceRequests = LowerLimit( m_settings.maxNamespaceRequests, peerLimits.maxNamespaceRequests );
        info.maxNamespaceListings = LowerLimit( m_settings.maxNamespaceListings, peerLimits.maxNamespaceListings );
        info.maxUploadBytesPerSecond = LowerLimit( m_settings.maxUploadBytesPerSecond, peerLimits.maxUploadBytesPerSecond );
    }

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    CDRNNamespaceCodec::WriteAnswerHeader( writer, requestId, DRN_RES_NS_HELLO, DRN_RESSTATUS_OK, CORE::CString::Empty );
    CDRNNamespaceCodec::WriteHelloBody( writer, info );
    ++m_stats.namespaceRequestsServed;
    Send( peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_NS_ANSWER, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnNsShares( UInt32 peerId, UInt32 requestId )
{GUCEF_TRACE;

    const CDRNAuthResult& auth = m_session->GetMemberAuthResult( peerId );
    CIDRNAccessPolicy& policy = m_session->GetNode().GetAccessPolicy();
    UInt32 budget = GetNsAnswerBytes() - NsAnswerHeaderReserve;

    m_nsBuffer.SetDataSize( 0 );
    CDRNWireWriter shares( m_nsBuffer );
    UInt32 count = 0;
    for ( TProviderMap::const_iterator p=m_providers.begin(); p!=m_providers.end(); ++p )
    {
        const CProviderEntry& entry = *(*p).second;

        // what a member may see: its role, and the policy for the root of the share
        if ( !entry.rules.AllowsBrowsing( auth.role, CORE::CString::Empty ) )
            continue;
        if ( !policy.MayBrowseResourceAs( peerId, auth, entry.name, CORE::CString::Empty, true ) )
            continue;
        if ( shares.GetOffset() + NsTextBytes( entry.name ) + 32 > budget )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceSessionComponent: Too many shares for one answer, \"" << entry.name << "\" and the ones after it are not listed" );
            break;
        }
        CDRNNamespaceShareInfo share;
        share.name = entry.name;
        share.capabilities = DRN_NS_CAP_READ | ( entry.provider->GetNamespaceCapabilities() & ( DRN_NS_CAP_LIST | DRN_NS_CAP_STAT ) );
        share.flags = entry.provider->GetNamespaceFlags() | DRN_NS_SHAREFLAG_READ_ONLY;
        UInt32 shareStat = 0;
        UInt32 sharePage = 0;
        entry.provider->GetNamespaceLimits( shareStat, sharePage );
        share.maxStatPaths = shareStat;
        share.maxPageEntries = sharePage;
        CDRNNamespaceCodec::WriteShare( shares, share );
        ++count;
    }

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    CDRNNamespaceCodec::WriteAnswerHeader( writer, requestId, DRN_RES_NS_SHARES, DRN_RESSTATUS_OK, CORE::CString::Empty );
    writer.WriteVarUInt32( count );
    if ( shares.GetOffset() > 0 )
        writer.WriteBytes( m_nsBuffer.GetConstBufferPtr(), shares.GetOffset() );
    ++m_stats.namespaceRequestsServed;
    Send( peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_NS_ANSWER, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnNsStat( UInt32 peerId, UInt32 requestId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 id = 0;
    UInt32 count = 0;
    CORE::CString share;
    if ( !CDRNNamespaceCodec::ReadStatRequestHeader( reader, id, share, count ) )
    {
        ++m_stats.namespaceMalformed;
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed request" );
        return;
    }
    CORE::CString busyWhy;
    CDRNPeerLimits peerLimits = GetPeerLimits( peerId );
    if ( IsLimitReached( (UInt32) m_nsServing.size(), m_settings.maxNamespaceRequests, 0 != peerLimits.maxNamespaceRequests ? CountNsServing( peerId ) : 0,
                         peerLimits.maxNamespaceRequests, "requests under way", busyWhy ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_BUSY, busyWhy );
        return;
    }
    TProviderMap::iterator p = m_providers.find( share );
    if ( p == m_providers.end() )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_NO_SUCH_PROVIDER, "no such share" );
        return;
    }
    CProviderEntry& entry = *(*p).second;
    if ( 0 == ( entry.provider->GetNamespaceCapabilities() & DRN_NS_CAP_STAT ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_UNSUPPORTED, "the share has no namespace" );
        return;
    }
    if ( count > GetNsStatLimit( *entry.provider ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_TOO_LARGE, "too many paths in one request" );
        return;
    }

    // every path needs at least its two length bytes: a count the message cannot hold is not read
    if ( (UInt64) count * 2 > reader.GetRemaining() )
    {
        ++m_stats.namespaceMalformed;
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed request" );
        return;
    }
    CNsStatJob* job = GUCEF_NEW CNsStatJob();
    CIDRNResourceIoJobPtr jobPtr( job );
    job->provider = entry.provider;
    job->paths.resize( count );
    job->todo.resize( count, 0 );
    job->results.resize( count );
    CNsServe* serve = GUCEF_NEW CNsServe();
    serve->ifRevisions.resize( count );
    serve->presets.resize( count );
    const CDRNAuthResult& auth = m_session->GetMemberAuthResult( peerId );
    CIDRNAccessPolicy& policy = m_session->GetNode().GetAccessPolicy();
    UInt32 todoCount = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !CDRNNamespaceCodec::ReadStatRequestItem( reader, job->paths[ i ], serve->ifRevisions[ i ] ) )
        {
            ++m_stats.namespaceMalformed;
            GUCEF_DELETE serve;
            RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed request" );
            return;
        }
        const CORE::CString& path = job->paths[ i ];
        CDRNNamespaceEntry& preset = serve->presets[ i ];
        preset.name = path;

        // names are checked here, before a provider sees them; the share rules and the policy decide before anything is resolved
        if ( !path.IsNULLOrEmpty() && !CDRNResourceName::IsValidResourceName( path ) )
        {
            preset.status = DRN_RESSTATUS_INVALID_NAME;
            continue;
        }
        bool shared = entry.rules.AllowsBrowsing( auth.role, path );
        if ( !shared || !policy.MayBrowseResourceAs( peerId, auth, share, path, shared ) )
        {
            preset.status = DRN_RESSTATUS_FORBIDDEN;
            continue;
        }
        if ( IsHiddenByPolicy( peerId, auth, share, path, true ) )
        {
            preset.status = DRN_RESSTATUS_NOT_FOUND;
            continue;
        }
        job->todo[ i ] = 1;
        ++todoCount;
    }
    serve->peerId = peerId;
    serve->requestId = requestId;
    serve->kind = DRN_RES_NS_STAT;
    serve->share = share;
    serve->provider = entry.provider;
    serve->job = jobPtr;
    if ( 0 == todoCount )
    {
        // nothing for the provider to do: answered at once through the same path
        m_nsServing.push_back( serve );
        serve->future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        CompleteNsStat( *serve );
        return;
    }
    serve->future = CDRNResourceIoTaskConsumer::Queue( m_settings.threadPoolName, serve->job, m_session->GetNode().GetWorkPulseGenerator() );
    if ( serve->future.HasNoFuture() )
    {
        GUCEF_DELETE serve;
        RefuseNs( peerId, requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_BUSY, "no thread pool to work on" );
        return;
    }
    m_nsServing.push_back( serve );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::CompleteNsStat( CNsServe& serve )
{GUCEF_TRACE;

    CNsStatJob* job = static_cast< CNsStatJob* >( serve.job.GetPointerAlways() );
    const CDRNAuthResult& auth = m_session->GetMemberAuthResult( serve.peerId );
    CIDRNAccessPolicy& policy = m_session->GetNode().GetAccessPolicy();
    UInt32 count = (UInt32) job->paths.size();

    m_nsBuffer.SetDataSize( 0 );
    CDRNWireWriter items( m_nsBuffer );
    for ( UInt32 i=0; i<count; ++i )
    {
        CDRNNamespaceEntry& entry = 0 != job->todo[ i ] ? job->results[ i ] : serve.presets[ i ];
        if ( 0 != job->todo[ i ] && DRN_RESSTATUS_OK == entry.status )
        {
            if ( NsTextBytes( entry.revision ) > CDRNNamespaceEntry::MaxRevisionBytes || ( DRN_NS_KIND_FILE != entry.kind && DRN_NS_KIND_DIRECTORY != entry.kind ) )
            {
                entry.status = DRN_RESSTATUS_IO_ERROR;
            }
            else
            if ( !job->paths[ i ].IsNULLOrEmpty() && !policy.MayListEntryAs( serve.peerId, auth, serve.share, job->paths[ i ], entry.IsDirectory() ) )
            {
                entry.status = DRN_RESSTATUS_NOT_FOUND;       // hidden: as if it did not exist
            }
            else
            if ( !serve.ifRevisions[ i ].IsNULLOrEmpty() && serve.ifRevisions[ i ] == entry.revision )
            {
                entry.status = DRN_RESSTATUS_UNCHANGED;
            }
        }
        if ( DRN_RESSTATUS_OK == entry.status )
            ++m_stats.namespaceEntriesServed;
        CDRNNamespaceCodec::WriteStatItem( items, entry );
    }

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    CDRNNamespaceCodec::WriteAnswerHeader( writer, serve.requestId, DRN_RES_NS_STAT, DRN_RESSTATUS_OK, CORE::CString::Empty );
    writer.WriteVarUInt32( count );
    if ( items.GetOffset() > 0 )
        writer.WriteBytes( m_nsBuffer.GetConstBufferPtr(), items.GetOffset() );
    ++m_stats.namespaceRequestsServed;
    Send( serve.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_NS_ANSWER, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
    DropNsServe( &serve );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnNsList( UInt32 peerId, UInt32 requestId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 id = 0;
    UInt32 maxEntries = 0;
    CORE::CString share;
    CORE::CString dir;
    CORE::CString cursor;
    CORE::CString ifRevisionNot;
    if ( !CDRNNamespaceCodec::ReadListRequest( reader, id, share, dir, cursor, maxEntries, ifRevisionNot ) )
    {
        ++m_stats.namespaceMalformed;
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed request" );
        return;
    }
    CORE::CString busyWhy;
    CDRNPeerLimits peerLimits = GetPeerLimits( peerId );
    if ( IsLimitReached( (UInt32) m_nsServing.size(), m_settings.maxNamespaceRequests, 0 != peerLimits.maxNamespaceRequests ? CountNsServing( peerId ) : 0,
                         peerLimits.maxNamespaceRequests, "requests under way", busyWhy ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_BUSY, busyWhy );
        return;
    }
    TProviderMap::iterator p = m_providers.find( share );
    if ( p == m_providers.end() )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_NO_SUCH_PROVIDER, "no such share" );
        return;
    }
    CProviderEntry& entry = *(*p).second;
    if ( 0 == ( entry.provider->GetNamespaceCapabilities() & DRN_NS_CAP_LIST ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_UNSUPPORTED, "the share has no namespace" );
        return;
    }
    if ( !dir.IsNULLOrEmpty() && !CDRNResourceName::IsValidResourceName( dir ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_INVALID_NAME, "not a valid directory name" );
        return;
    }
    UInt32 pageLimit = GetNsPageLimit( *entry.provider );
    if ( maxEntries > pageLimit )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_TOO_LARGE, "more entries in a page than the serving side allows" );
        return;
    }
    if ( 0 == maxEntries )
        maxEntries = pageLimit;
    if ( NsTextBytes( ifRevisionNot ) > CDRNNamespaceEntry::MaxRevisionBytes )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed request" );
        return;
    }

    // the rules and the policy decide before anything is resolved
    const CDRNAuthResult& auth = m_session->GetMemberAuthResult( peerId );
    bool shared = entry.rules.AllowsBrowsing( auth.role, dir );
    if ( !shared || !m_session->GetNode().GetAccessPolicy().MayBrowseResourceAs( peerId, auth, share, dir, shared ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_FORBIDDEN, "not shared with this member" );
        return;
    }
    if ( IsHiddenByPolicy( peerId, auth, share, dir, true ) )
    {
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_NOT_FOUND, "no such directory" );
        return;
    }

    // a cursor continues a listing of this member; a first request starts one
    CNsListing* listing = GUCEF_NULL;
    bool newListing = cursor.IsNULLOrEmpty();
    if ( !newListing )
    {
        UInt32 listingId = 0;
        UInt32 seq = 0;
        UInt32 size = NsTextBytes( cursor );
        const char* text = cursor.C_String();
        UInt32 dot = 0;
        while ( dot < size && '.' != text[ dot ] )
            ++dot;
        bool parsed = dot > 0 && dot + 1 < size && size < 24;
        for ( UInt32 i=0; i<size && parsed; ++i )
            parsed = i == dot || ( text[ i ] >= '0' && text[ i ] <= '9' );
        if ( !parsed )
        {
            ++m_stats.namespaceMalformed;
            RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_PROTOCOL_ERROR, "malformed cursor" );
            return;
        }
        listingId = CORE::StringToUInt32( CORE::CString( text, dot ) );
        seq = CORE::StringToUInt32( CORE::CString( text + dot + 1, size - dot - 1 ) );
        listing = FindNsListing( peerId, listingId );
        if ( GUCEF_NULL != listing && !listing->busy && 0 != m_settings.namespaceListingLifetimeMs &&
             CDRNClock::NowInMilliseconds() >= listing->lastActivityMs + m_settings.namespaceListingLifetimeMs )
        {
            // past its lifetime, whether or not the tick got to it yet
            ++m_stats.namespaceListingsExpired;
            DropNsListing( listing );
            listing = GUCEF_NULL;
        }
        if ( GUCEF_NULL == listing || listing->share != share || listing->dir != dir || listing->nextSeq != seq )
        {
            RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_CURSOR_EXPIRED, "the listing is not known (any more): list again from the start" );
            return;
        }
        if ( listing->busy )
        {
            RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_BUSY, "the listing is being read" );
            return;
        }
    }
    else
    {
        UInt32 open = 0;
        for ( UInt32 i=0; i<m_nsListings.size(); ++i )
            if ( m_nsListings[ i ]->peerId == peerId )
                ++open;
        if ( IsLimitReached( (UInt32) m_nsListings.size(), m_settings.maxNamespaceListings, open, peerLimits.maxNamespaceListings, "listings under way", busyWhy ) )
        {
            RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_BUSY, busyWhy );
            return;
        }
        listing = GUCEF_NEW CNsListing();
        listing->peerId = peerId;
        listing->id = m_nextNsListingId++;
        if ( 0 == m_nextNsListingId )
            m_nextNsListingId = 1;
        listing->share = share;
        listing->dir = dir;
        listing->provider = entry.provider;
        listing->nextSeq = 0;
        m_nsListings.push_back( listing );
        ++m_stats.namespaceListingsOpened;
    }
    listing->lastActivityMs = CDRNClock::NowInMilliseconds();

    CNsServe* serve = GUCEF_NEW CNsServe();
    serve->peerId = peerId;
    serve->requestId = requestId;
    serve->kind = DRN_RES_NS_LIST;
    serve->share = share;
    serve->provider = entry.provider;
    serve->listing = listing;
    serve->newListing = newListing;
    serve->maxEntries = maxEntries;
    serve->dir = dir;

    // entries read earlier and not sent fill the page first
    UInt32 carried = (UInt32) listing->carry.size();
    UInt32 want = maxEntries > carried ? maxEntries - carried : 0;
    if ( !newListing && ( 0 == want || listing->atEnd ) )
    {
        m_nsServing.push_back( serve );
        listing->busy = true;
        CNsListJob* job = GUCEF_NEW CNsListJob();
        job->provider = entry.provider;
        job->status = DRN_RESSTATUS_OK;
        job->atEnd = listing->atEnd;
        serve->job = CIDRNResourceIoJobPtr( job );
        serve->future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        CompleteNsList( *serve );
        return;
    }
    CNsListJob* job = GUCEF_NEW CNsListJob();
    job->provider = entry.provider;
    job->dirName = dir;
    job->ifRevisionNot = ifRevisionNot;
    job->enumerator = listing->enumerator;
    job->want = want;
    serve->job = CIDRNResourceIoJobPtr( job );
    serve->future = CDRNResourceIoTaskConsumer::Queue( m_settings.threadPoolName, serve->job, m_session->GetNode().GetWorkPulseGenerator() );
    if ( serve->future.HasNoFuture() )
    {
        GUCEF_DELETE serve;
        if ( newListing )
            DropNsListing( listing );
        RefuseNs( peerId, requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_BUSY, "no thread pool to work on" );
        return;
    }
    listing->busy = true;
    m_nsServing.push_back( serve );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::FinishNsList( CNsServe& serve, TDRNResourceStatus status, const CORE::CString& message, bool unchanged )
{GUCEF_TRACE;

    CNsListing* listing = serve.listing;
    CNsListJob* job = static_cast< CNsListJob* >( serve.job.GetPointerAlways() );
    ++m_stats.namespaceRequestsServed;
    if ( unchanged )
    {
        m_txBuffer.SetDataSize( 0 );
        CDRNWireWriter writer( m_txBuffer );
        CDRNNamespaceCodec::WriteAnswerHeader( writer, serve.requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_UNCHANGED, CORE::CString::Empty );
        CDRNNamespaceCodec::WriteText( writer, job->dirEntry.revision );
        Send( serve.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_NS_ANSWER, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
    }
    else
    {
        SendNsAnswer( serve.peerId, serve.requestId, DRN_RES_NS_LIST, status, message );
    }
    DropNsListing( listing );
    DropNsServe( &serve );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::CompleteNsList( CNsServe& serve )
{GUCEF_TRACE;

    CNsListJob* job = static_cast< CNsListJob* >( serve.job.GetPointerAlways() );
    CNsListing* listing = serve.listing;
    TDRNResourceStatus status = job->status;
    if ( DRN_RESSTATUS_UNCHANGED == status )
    {
        FinishNsList( serve, status, CORE::CString::Empty, true );
        return;
    }
    if ( DRN_RESSTATUS_OK != status )
    {
        const char* text = DRN_RESSTATUS_NOT_FOUND == status ? "no such directory" : DRN_RESSTATUS_NOT_A_DIRECTORY == status ? "not a directory" : "the directory cannot be read";
        FinishNsList( serve, status, text, false );
        return;
    }
    if ( serve.newListing )
    {
        listing->enumerator = job->enumerator;
        listing->dirRevision = job->dirEntry.revision;
        if ( NsTextBytes( listing->dirRevision ) > CDRNNamespaceEntry::MaxRevisionBytes )
            listing->dirRevision.Clear();
    }
    if ( !serve.newListing && !job->enumerator.IsNULL() )
        listing->enumerator = job->enumerator;
    listing->atEnd = job->atEnd;

    const CDRNAuthResult& auth = m_session->GetMemberAuthResult( serve.peerId );
    CIDRNAccessPolicy& policy = m_session->GetNode().GetAccessPolicy();
    CProviderEntry* providerEntry = GUCEF_NULL;
    TProviderMap::iterator p = m_providers.find( serve.share );
    if ( p != m_providers.end() )
        providerEntry = (*p).second;
    if ( GUCEF_NULL == providerEntry )
    {
        FinishNsList( serve, DRN_RESSTATUS_CLOSED, "the provider is no longer served", false );
        return;
    }
    const CDRNResourceShareRules& rules = providerEntry->rules;

    UInt32 budget = GetNsAnswerBytes() - NsAnswerHeaderReserve;
    m_nsBuffer.SetDataSize( 0 );
    CDRNWireWriter entries( m_nsBuffer );
    UInt32 used = 0;
    GUCEF::vector< CDRNNamespaceEntry > carry;
    CORE::CString fullName;
    UInt32 carried = (UInt32) listing->carry.size();
    for ( UInt32 i=0; i<carried + job->entries.size(); ++i )
    {
        bool fromCarry = i < carried;
        const CDRNNamespaceEntry& entry = fromCarry ? listing->carry[ i ] : job->entries[ i - carried ];
        if ( !fromCarry )
        {
            // what the provider listed is checked, not trusted: only names the resource rules allow, only what the member may see
            UInt32 nameBytes = NsTextBytes( entry.name );
            if ( entry.name.IsNULLOrEmpty() || !CDRNResourceName::IsValidResourceName( entry.name ) || entry.name.HasChar( '/' ) >= 0 ||
                 NsTextBytes( serve.dir ) + 1 + nameBytes > CDRNResourceName::MaxResourceNameLength ||
                 NsTextBytes( entry.revision ) > CDRNNamespaceEntry::MaxRevisionBytes || ( DRN_NS_KIND_FILE != entry.kind && DRN_NS_KIND_DIRECTORY != entry.kind ) )
            {
                ++m_stats.namespaceEntriesInvalidName;
                continue;
            }
            if ( serve.dir.IsNULLOrEmpty() )
            {
                fullName = entry.name;
            }
            else
            {
                fullName = serve.dir;
                fullName += "/";
                fullName += entry.name;
            }
            bool shows = rules.ShowsEntry( fullName, entry.IsDirectory() );
            if ( !shows || !policy.MayBrowseResourceAs( serve.peerId, auth, serve.share, fullName, shows ) ||
                 !policy.MayListEntryAs( serve.peerId, auth, serve.share, fullName, entry.IsDirectory() ) )
            {
                ++m_stats.namespaceEntriesHidden;
                continue;
            }
        }
        UInt32 estimate = NsTextBytes( entry.name ) + NsTextBytes( entry.revision ) + 80;
        if ( used >= serve.maxEntries || ( used > 0 && entries.GetOffset() + estimate > budget ) )
        {
            carry.push_back( entry );
            continue;
        }
        CDRNNamespaceCodec::WriteListEntry( entries, entry );
        ++used;
    }
    m_stats.namespaceEntriesServed += used;
    listing->carry.swap( carry );
    bool done = listing->atEnd && listing->carry.empty();

    CORE::CString nextCursor;
    if ( !done )
        nextCursor = CORE::ToString( listing->id ) + "." + CORE::ToString( listing->nextSeq + 1 );
    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    CDRNNamespaceCodec::WriteAnswerHeader( writer, serve.requestId, DRN_RES_NS_LIST, DRN_RESSTATUS_OK, CORE::CString::Empty );
    CDRNNamespaceCodec::WriteListHeader( writer, listing->dirRevision, nextCursor, used );
    if ( entries.GetOffset() > 0 )
        writer.WriteBytes( m_nsBuffer.GetConstBufferPtr(), entries.GetOffset() );
    ++m_stats.namespaceRequestsServed;
    Send( serve.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_NS_ANSWER, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );

    if ( done )
    {
        DropNsListing( listing );
    }
    else
    {
        ++listing->nextSeq;
        listing->busy = false;
        listing->lastActivityMs = CDRNClock::NowInMilliseconds();
    }
    DropNsServe( &serve );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::TickNamespace( UInt64 nowMs )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_nsServing.size(); )
    {
        CNsServe* serve = m_nsServing[ i ];
        if ( !serve->job.IsNULL() && serve->future.IsReady() )
        {
            UInt32 before = (UInt32) m_nsServing.size();
            if ( DRN_RES_NS_STAT == serve->kind )
                CompleteNsStat( *serve );
            else
                CompleteNsList( *serve );
            if ( m_nsServing.size() != before )
            {
                i = 0;          // the request went (and maybe more): start over
                continue;
            }
        }
        ++i;
    }

    // listings a member did not continue in time are forgotten: its next request with the cursor starts it over
    if ( 0 != m_settings.namespaceListingLifetimeMs )
    {
        for ( UInt32 i=0; i<m_nsListings.size(); )
        {
            CNsListing* listing = m_nsListings[ i ];
            if ( !listing->busy && nowMs >= listing->lastActivityMs + m_settings.namespaceListingLifetimeMs )
            {
                ++m_stats.namespaceListingsExpired;
                GUCEF_DELETE listing;
                m_nsListings.erase( m_nsListings.begin() + i );
                continue;
            }
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/
//  Namespace: requesting
/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNextNamespaceRequestId( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    UInt32 id = m_nextNsRequestId++;
    if ( 0 == m_nextNsRequestId )
        m_nextNsRequestId = 1;
    return id;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceSessionComponent::GetNamespaceRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return (UInt32) m_nsPending.size();
}

/*-------------------------------------------------------------------------*/

/** What a thread that pumps the node waits for in CallNamespace(): the answer of the request */
class CNamespaceAnswerCondition : public CIDRNWaitCondition
{
    public:

    explicit CNamespaceAnswerCondition( CDRNNamespaceRequest& request ) : m_request( request ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_request.IsDone();
    }

    private:

    CNamespaceAnswerCondition& operator=( const CNamespaceAnswerCondition& src );
    CDRNNamespaceRequest& m_request;
};

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNResourceSessionComponent::CallNamespace( UInt32 peerId, UInt32 opcode, UInt32 requestId, const CORE::CDynamicBuffer& requestBody,
                                             UInt32 timeoutInMs, CORE::CString& message, CORE::CDynamicBuffer& answerBody )
{GUCEF_TRACE;

    answerBody.SetDataSize( 0 );
    message.Clear();
    if ( m_session->GetNode().IsWorkingOnThisThread() )
    {
        message = "a wait on a thread that works on the node would never end";
        return DRN_RESSTATUS_TIMEOUT;
    }

    CDRNNamespaceRequestPtr request;
    {
        MT::CScopeMutex lock( m_session->GetNode().GetLock() );
        TDRNResourceStatus status = DRN_RESSTATUS_OK;
        if ( GetUsableLink( peerId, status, message, DRN_PROTOCOL_MINOR_NAMESPACE ).IsNULL() )
            return status;
        request = CDRNNamespaceRequestPtr( GUCEF_NEW CDRNNamespaceRequest( peerId, requestId, opcode ) );
        m_nsPending.push_back( request );
        if ( !Send( peerId, DRN_LANE_RELIABLE_ORDERED, opcode, requestBody.GetConstBufferPtr(), requestBody.GetDataSize() ) )
        {
            m_nsPending.pop_back();
            message = "the request could not be sent";
            return DRN_RESSTATUS_LINK_LOST;
        }
        ++m_stats.namespaceRequestsMade;
        if ( m_session->GetNode().IsRunningOwnThread() )
            m_session->GetNode().WakeOwnThread();
    }

    // the thread that pumps the node works the node itself while it waits (nobody else would), any other thread waits
    bool answered = false;
    if ( m_session->GetNode().IsPumpThread() )
    {
        CNamespaceAnswerCondition condition( *request.GetPointerAlways() );
        answered = m_session->GetNode().PumpUntil( condition, timeoutInMs );
    }
    else
    {
        answered = request->Wait( timeoutInMs );
    }
    if ( !answered )
    {
        // given up: a late answer finds nobody and is dropped
        MT::CScopeMutex lock( m_session->GetNode().GetLock() );
        for ( UInt32 i=0; i<m_nsPending.size(); ++i )
        {
            if ( m_nsPending[ i ].GetPointerAlways() == request.GetPointerAlways() )
            {
                m_nsPending.erase( m_nsPending.begin() + i );
                break;
            }
        }
        if ( !request->IsDone() )
        {
            message = "no answer in time";
            return DRN_RESSTATUS_TIMEOUT;
        }
    }
    message = request->GetMessage();
    TDRNResourceStatus status = request->GetStatus();
    const CORE::CDynamicBuffer& body = request->GetBody();
    if ( body.GetDataSize() > 0 && answerBody.SetBufferSize( body.GetDataSize(), false, true ) )
    {
        answerBody.SetDataSize( body.GetDataSize() );
        memcpy( answerBody.GetBufferPtr(), body.GetConstBufferPtr(), body.GetDataSize() );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::OnNsAnswer( UInt32 peerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 requestId = 0;
    UInt32 kind = 0;
    TDRNResourceStatus status = DRN_RESSTATUS_OK;
    CORE::CString message;
    if ( !CDRNNamespaceCodec::ReadAnswerHeader( reader, requestId, kind, status, message ) )
    {
        ++m_stats.namespaceAnswersDropped;
        ++m_stats.droppedMessages;
        return;
    }
    for ( UInt32 i=0; i<m_nsPending.size(); ++i )
    {
        CDRNNamespaceRequestPtr request = m_nsPending[ i ];
        if ( request->GetPeerId() == peerId && request->GetRequestId() == requestId )
        {
            if ( request->GetKind() != kind )
                break;      // not the answer of that request: dropped
            m_nsPending.erase( m_nsPending.begin() + i );
            ++m_stats.namespaceAnswersReceived;
            request->Complete( status, message, reader.GetCurrentPtr(), reader.GetRemaining() );
            return;
        }
    }
    ++m_stats.namespaceAnswersDropped;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceSessionComponent::FailNamespaceRequestsOf( UInt32 peerId, bool allPeers, TDRNResourceStatus status, const CORE::CString& why )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_nsPending.size(); )
    {
        CDRNNamespaceRequestPtr request = m_nsPending[ i ];
        if ( allPeers || request->GetPeerId() == peerId )
        {
            m_nsPending.erase( m_nsPending.begin() + i );
            request->Fail( status, why );
            continue;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
