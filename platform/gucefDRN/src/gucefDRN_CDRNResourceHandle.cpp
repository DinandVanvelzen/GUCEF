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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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

#ifndef GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#include "gucefDRN_CDRNResourceSessionComponent.h"
#define GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H ? */

#include "gucefDRN_CDRNResourceHandle.h"

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

/** A chunk of the window under way, as it arrived */
class CDRNResourceHandle::CArrived
{
    public:

    CORE::CDynamicBuffer data;
    UInt32 pathLength;
    UInt8 path[ CDRNResourceHashTree::MaxAuditPathLength * CDRNResourceHashTree::HashSize ];

    CArrived( void ) : data(), pathLength( 0 ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourceHandle::CDRNResourceHandle( CDRNResourceSessionComponent& component, UInt32 peerId, UInt32 handleId,
                                        const CORE::CString& providerName, const CORE::CString& resourceName,
                                        const CDRNResourceTransferOptions& options )
    : m_lock()
    , m_signal( 0 )
    , m_component( &component )
    , m_node( &component.GetSession().GetNode() )
    , m_peerId( peerId )
    , m_handleId( handleId )
    , m_providerName( providerName )
    , m_resourceName( resourceName )
    , m_options( options )
    , m_state( STATE_OPENING )
    , m_result()
    , m_info()
    , m_useTree( false )
    , m_readId( 0 )
    , m_nextReadId( 1 )
    , m_readFirst( 0 )
    , m_readCount( 0 )
    , m_readOutstanding( 0 )
    , m_readStatus( DRN_RESSTATUS_OK )
    , m_readMessage()
    , m_arrived()
    , m_readCountTotal( 0 )
    , m_openCount( 0 )
    , m_hasher()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceHandle::~CDRNResourceHandle()
{GUCEF_TRACE;

    Close();
    MT::CScopeMutex lock( m_lock );
    ClearArrived();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::ClearArrived( void )
{GUCEF_TRACE;

    for ( TArrivedMap::iterator i=m_arrived.begin(); i!=m_arrived.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_arrived.clear();
}

/*-------------------------------------------------------------------------*/

CDRNResourceHandle::TState
CDRNResourceHandle::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHandle::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return STATE_OPEN == m_state;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceHandle::GetResult( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_result;
}

/*-------------------------------------------------------------------------*/

CDRNResourceInfo
CDRNResourceHandle::GetInfo( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_info;
}

/*-------------------------------------------------------------------------*/

const CDRNResourceTransferOptions&
CDRNResourceHandle::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceHandle::GetPeerId( void ) const
{GUCEF_TRACE;

    return m_peerId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourceHandle::GetProviderName( void ) const
{GUCEF_TRACE;

    return m_providerName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourceHandle::GetResourceName( void ) const
{GUCEF_TRACE;

    return m_resourceName;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceHandle::GetReadCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_readCountTotal;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceHandle::GetOpenCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_openCount;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHandle::SendOpen( void )
{GUCEF_TRACE;

    // node lock held
    CORE::CString expectedRevision = m_options.expectedRevision;
    bool wantTree = m_options.WantsTree();
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_openCount > 0 )
        {
            // opened again: for the revision it had
            expectedRevision = m_info.revision;
            wantTree = wantTree || m_useTree;
        }
        ++m_openCount;
        m_state = STATE_OPENING;
    }
    m_component->m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_component->m_txBuffer );
    writer.WriteVarUInt32( m_handleId );
    writer.WriteString( m_providerName );
    writer.WriteString( m_resourceName );
    writer.WriteVarUInt32( wantTree ? DRN_RES_OPEN_FLAG_TREE : 0 );
    writer.WriteString( expectedRevision );
    return m_component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_OPEN, m_component->m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::OnInfo( TDRNResourceStatus status, const CORE::CString& message, const CDRNResourceInfo& info )
{GUCEF_TRACE;

    // node lock held
    bool closeIt = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_OPENING != m_state )
            return;
        if ( DRN_RESSTATUS_OK != status )
        {
            m_state = STATE_CLOSED;
            m_result = CDRNResourceTransferResult( status, message );
        }
        else
        if ( m_options.RequiresTree() && !info.hasTreeRoot )
        {
            m_state = STATE_CLOSED;
            m_result = CDRNResourceTransferResult( DRN_RESSTATUS_NO_TREE, "the serving side has no hash tree of the resource" );
            closeIt = true;
        }
        else
        {
            bool digestOk = true;
            if ( m_options.hasTreeDigest )
            {
                UInt8 digest[ CDRNResourceInfo::DigestSize ];
                CDRNResourceHasher hasher;
                CDRNResourceHashTree::ComputeDigest( hasher, info.size, info.treeRoot, digest );
                digestOk = CORE::CSha256::ConstantTimeEquals( digest, m_options.treeDigest, sizeof( digest ) );
            }
            if ( !digestOk )
            {
                m_state = STATE_CLOSED;
                m_result = CDRNResourceTransferResult( DRN_RESSTATUS_DIGEST_MISMATCH, "the resource does not have the digest that was expected" );
                closeIt = true;
            }
            else
            {
                m_info = info;
                m_info.name = m_resourceName;
                m_useTree = info.hasTreeRoot && m_options.WantsTree();
                m_state = STATE_OPEN;
                m_result = CDRNResourceTransferResult();
            }
        }
    }
    if ( closeIt )
    {
        UInt8 body[ 8 ];
        UInt32 size = CDRNWireWriter::EncodeVarUInt64( m_handleId, body );
        m_component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_CLOSE, body, size );
    }
    m_signal.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::OnData( UInt32 readId, UInt64 chunk, const UInt8* path, UInt32 pathLength, const UInt8* data, UInt32 dataSize )
{GUCEF_TRACE;

    // node lock held
    bool complete = false;
    bool taken = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_OPEN == m_state && 0 != m_readId && readId == m_readId && chunk >= m_readFirst && chunk < m_readFirst + m_readCount &&
             m_arrived.find( chunk ) == m_arrived.end() && m_readOutstanding > 0 && dataSize == m_info.GetChunkBytes( chunk ) )
        {
            CArrived* arrived = GUCEF_NEW CArrived();
            arrived->data.CopyFrom( dataSize, data );
            arrived->pathLength = pathLength;
            if ( pathLength > 0 )
                memcpy( arrived->path, path, pathLength * CDRNResourceHashTree::HashSize );
            m_arrived[ chunk ] = arrived;
            --m_readOutstanding;
            taken = true;
            complete = m_arrived.size() >= m_readCount;
        }
    }
    if ( taken )
    {
        m_component->AddLinkOutstanding( m_peerId, -1 );
        ++m_component->m_stats.chunksReceived;
        m_component->m_stats.bytesReceived += dataSize;
    }
    else
    {
        ++m_component->m_stats.droppedMessages;
    }
    if ( complete )
        m_signal.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::OnReadEnd( UInt32 readId, TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    // node lock held
    UInt32 outstanding = 0;
    {
        MT::CScopeMutex lock( m_lock );
        if ( 0 == m_readId || readId != m_readId )
            return;
        m_readStatus = DRN_RESSTATUS_OK == status ? DRN_RESSTATUS_PROTOCOL_ERROR : status;
        m_readMessage = message;
        outstanding = m_readOutstanding;
        m_readOutstanding = 0;
    }
    m_component->AddLinkOutstanding( m_peerId, -(Int64) outstanding );
    m_signal.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::OnLost( TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    // node lock held
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_CLOSED == m_state )
            return;
        m_state = STATE_CLOSED;
        m_result = CDRNResourceTransferResult( status, message );
        m_readOutstanding = 0;
    }
    m_signal.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::Detach( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_component = GUCEF_NULL;
        m_node = GUCEF_NULL;
        if ( STATE_CLOSED != m_state )
        {
            m_state = STATE_CLOSED;
            m_result = CDRNResourceTransferResult( DRN_RESSTATUS_LINK_LOST, "the session went away" );
        }
    }
    m_signal.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHandle::Close( void )
{GUCEF_TRACE;

    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
    }
    if ( GUCEF_NULL == node )
        return;

    MT::CScopeMutex nodeLock( node->GetLock() );
    bool tellServer = false;
    CDRNResourceSessionComponent* component = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        component = m_component;
        if ( GUCEF_NULL == component )
            return;
        tellServer = STATE_CLOSED != m_state || DRN_RESSTATUS_LINK_LOST != m_result.status;
        if ( STATE_CLOSED != m_state )
        {
            m_state = STATE_CLOSED;
            m_result = CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "closed" );
        }
        m_component = GUCEF_NULL;
        m_node = GUCEF_NULL;
    }
    if ( tellServer )
    {
        UInt8 body[ 8 ];
        UInt32 size = CDRNWireWriter::EncodeVarUInt64( m_handleId, body );
        component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_CLOSE, body, size );
    }
    component->ForgetHandle( this );
    m_signal.Post();
}

/*-------------------------------------------------------------------------*/

/** What a thread that pumps the node waits for in WaitFor(): the handle open, or the window read, or closed */
class CDRNResourceHandle::CWaitCondition : public CIDRNWaitCondition
{
    public:

    CWaitCondition( const CDRNResourceHandle& handle, bool forOpen ) : m_handle( handle ), m_forOpen( forOpen ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_handle.IsWaitOver( m_forOpen );
    }

    private:

    CWaitCondition& operator=( const CWaitCondition& src );
    const CDRNResourceHandle& m_handle;
    bool m_forOpen;
};

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHandle::IsWaitOver( bool forOpen ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( STATE_CLOSED == m_state )
        return true;
    if ( forOpen && STATE_OPEN == m_state )
        return true;
    if ( !forOpen && ( DRN_RESSTATUS_OK != m_readStatus || m_arrived.size() >= m_readCount ) )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHandle::WaitFor( bool forOpen, UInt32 timeoutInMs )
{GUCEF_TRACE;

    // the thread that pumps the node works the node itself while it waits: the answers arrive in the cycles of the node
    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
    }
    if ( GUCEF_NULL != node && node->IsPumpThread() )
    {
        CWaitCondition condition( *this, forOpen );
        return node->PumpUntil( condition, timeoutInMs );
    }

    UInt64 deadline = CDRNClock::NowInMilliseconds() + timeoutInMs;
    while ( true )
    {
        if ( IsWaitOver( forOpen ) )
            return true;
        UInt64 now = CDRNClock::NowInMilliseconds();
        if ( now >= deadline )
            return false;
        m_signal.WaitOrTimeout( (UInt32) ( deadline - now ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHandle::WaitOpen( UInt32 timeoutInMs )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL != m_node && m_node->IsWorkingOnThisThread() )
            return STATE_OPEN == m_state;
    }
    if ( !WaitFor( true, timeoutInMs ) )
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_OPENING == m_state )
            m_result = CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "the serving side did not open the resource in time" );
        return false;
    }
    return IsOpen();
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceHandle::Reopen( void )
{GUCEF_TRACE;

    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
    }
    if ( GUCEF_NULL == node )
        return CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "closed" );
    {
        MT::CScopeMutex nodeLock( node->GetLock() );
        CDRNResourceSessionComponent* component = GUCEF_NULL;
        {
            MT::CScopeMutex lock( m_lock );
            component = m_component;
        }
        if ( GUCEF_NULL == component )
            return CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "closed" );
        TDRNResourceStatus status = DRN_RESSTATUS_OK;
        CORE::CString why;
        if ( component->GetUsableLink( m_peerId, status, why ).IsNULL() || !SendOpen() )
            return CDRNResourceTransferResult( DRN_RESSTATUS_OK == status ? DRN_RESSTATUS_LINK_LOST : status, why );
        if ( node->IsRunningOwnThread() )
            node->WakeOwnThread();
    }
    if ( !WaitOpen( m_options.openTimeoutMs ) )
        return GetResult();
    return CDRNResourceTransferResult();
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceHandle::ReadWindow( UInt64 firstChunk, UInt32 count, GUCEF::vector< CORE::CDynamicBuffer >& chunks, bool& reopened )
{GUCEF_TRACE;

    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
        if ( STATE_OPEN != m_state )
            return STATE_CLOSED == m_state ? m_result : CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "not open" );
        if ( firstChunk >= m_info.GetChunkCount() || count > m_info.GetChunkCount() - firstChunk )
            return CDRNResourceTransferResult( DRN_RESSTATUS_OUT_OF_RANGE, "the window reaches beyond the end of the resource" );
    }
    if ( GUCEF_NULL == node )
        return CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "closed" );
    if ( node->IsWorkingOnThisThread() )
        return CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "a wait on a thread that works on the node would never end" );

    // the window is asked for in one READ with all its credit, within this node's receive limits
    UInt64 deadline = CDRNClock::NowInMilliseconds() + m_options.idleTimeoutMs;
    while ( true )
    {
        UInt32 delayMs = 0;
        {
            MT::CScopeMutex nodeLock( node->GetLock() );
            CDRNResourceSessionComponent* component = GUCEF_NULL;
            {
                MT::CScopeMutex lock( m_lock );
                component = m_component;
                if ( STATE_OPEN != m_state || GUCEF_NULL == component )
                    return STATE_CLOSED == m_state ? m_result : CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "closed" );
            }
            delayMs = component->AcquireReceiveBudget( m_peerId, count * DRN_RESOURCE_CHUNK_SIZE );
            if ( 0 == delayMs )
            {
                UInt32 readId = 0;
                {
                    MT::CScopeMutex lock( m_lock );
                    ClearArrived();
                    readId = m_nextReadId++;
                    if ( 0 == m_nextReadId )
                        m_nextReadId = 1;
                    m_readId = readId;
                    m_readFirst = firstChunk;
                    m_readCount = count;
                    m_readOutstanding = count;
                    m_readStatus = DRN_RESSTATUS_OK;
                    m_readMessage.Clear();
                    ++m_readCountTotal;
                }
                component->m_txBuffer.SetDataSize( 0 );
                CDRNWireWriter writer( component->m_txBuffer );
                writer.WriteVarUInt32( m_handleId );
                writer.WriteVarUInt32( readId );
                writer.WriteVarUInt64( firstChunk );
                writer.WriteVarUInt64( count );
                writer.WriteVarUInt32( count );
                if ( !component->Send( m_peerId, DRN_LANE_RELIABLE_ORDERED, DRN_RES_READ, component->m_txBuffer.GetConstBufferPtr(), writer.GetOffset() ) )
                    return CDRNResourceTransferResult( DRN_RESSTATUS_LINK_LOST, "the link went down" );
                component->AddLinkOutstanding( m_peerId, (Int64) count );
                if ( node->IsRunningOwnThread() )
                    node->WakeOwnThread();
                break;
            }
        }
        if ( CDRNClock::NowInMilliseconds() + delayMs > deadline )
            return CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "the receive limits leave no room in time" );
        MT::PrecisionDelay( delayMs );
    }

    bool done = WaitFor( false, m_options.idleTimeoutMs );

    TArrivedMap arrived;
    TDRNResourceStatus status = DRN_RESSTATUS_OK;
    CORE::CString message;
    CDRNResourceInfo info;
    bool useTree = false;
    UInt32 outstanding = 0;
    {
        MT::CScopeMutex lock( m_lock );
        arrived.swap( m_arrived );
        status = STATE_CLOSED == m_state ? m_result.status : m_readStatus;
        message = STATE_CLOSED == m_state ? m_result.message : m_readMessage;
        info = m_info;
        useTree = m_useTree;
        outstanding = m_readOutstanding;
        m_readOutstanding = 0;
        m_readId = 0;
    }
    if ( !done || DRN_RESSTATUS_OK != status )
    {
        // the read ends here: what is still on its way is dropped when it comes
        {
            MT::CScopeMutex nodeLock( node->GetLock() );
            CDRNResourceSessionComponent* component = GUCEF_NULL;
            {
                MT::CScopeMutex lock( m_lock );
                component = m_component;
            }
            if ( GUCEF_NULL != component && outstanding > 0 )
                component->AddLinkOutstanding( m_peerId, -(Int64) outstanding );
        }
        for ( TArrivedMap::iterator i=arrived.begin(); i!=arrived.end(); ++i )
            GUCEF_DELETE (*i).second;
        if ( !done )
            return CDRNResourceTransferResult( DRN_RESSTATUS_TIMEOUT, "no data arrived in time" );
        if ( DRN_RESSTATUS_CLOSED == status && !reopened )
        {
            // the serving side closed the handle for being idle: opened again once, for the same revision
            reopened = true;
            CDRNResourceTransferResult result = Reopen();
            if ( !result.IsOk() )
                return result;
            return ReadWindow( firstChunk, count, chunks, reopened );
        }
        return CDRNResourceTransferResult( status, message );
    }

    chunks.resize( count );
    CDRNResourceTransferResult result;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt64 index = firstChunk + i;
        TArrivedMap::iterator found = arrived.find( index );
        CArrived* entry = found != arrived.end() ? (*found).second : GUCEF_NULL;
        bool good = GUCEF_NULL != entry;
        if ( good && useTree )
        {
            UInt8 leaf[ CDRNResourceHashTree::HashSize ];
            m_hasher.HashLeaf( entry->data.GetConstBufferPtr(), entry->data.GetDataSize(), leaf );
            good = CDRNResourceHashTree::VerifyAuditPath( m_hasher, index, info.GetChunkCount(), leaf, entry->path, entry->pathLength, info.treeRoot );
        }
        if ( good )
        {
            chunks[ i ] = entry->data;
            continue;
        }

        // a chunk that failed its check is asked for again on its own, a few times
        bool repaired = false;
        for ( UInt32 attempt=0; attempt<m_options.corruptChunkRetries && !repaired; ++attempt )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceHandle: Chunk " << index << " of \"" << m_providerName << "/" << m_resourceName << "\" failed its check, asking again" );
            GUCEF::vector< CORE::CDynamicBuffer > single;
            bool singleReopened = reopened;
            CDRNResourceTransferResult retry = ReadWindowRaw( index, single, singleReopened );
            if ( !retry.IsOk() )
            {
                if ( DRN_RESSTATUS_DIGEST_MISMATCH == retry.status )
                    continue;
                result = retry;
                break;
            }
            chunks[ i ] = single[ 0 ];
            repaired = true;
        }
        if ( !repaired )
        {
            if ( result.IsOk() )
                result = CDRNResourceTransferResult( DRN_RESSTATUS_DIGEST_MISMATCH, "a chunk kept failing its check against the hash tree" );
            break;
        }
    }
    for ( TArrivedMap::iterator i=arrived.begin(); i!=arrived.end(); ++i )
        GUCEF_DELETE (*i).second;
    if ( !result.IsOk() )
        chunks.clear();
    return result;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceHandle::ReadWindowRaw( UInt64 chunk, GUCEF::vector< CORE::CDynamicBuffer >& single, bool& reopened )
{GUCEF_TRACE;

    // one chunk, checked: a window of one whose check failing is final for this attempt
    CDRNResourceTransferOptions saved = m_options;
    m_options.corruptChunkRetries = 0;
    CDRNResourceTransferResult result = ReadWindow( chunk, 1, single, reopened );
    m_options.corruptChunkRetries = saved.corruptChunkRetries;
    return result;
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceHandle::ReadChunks( UInt64 firstChunk, UInt32 count, GUCEF::vector< CORE::CDynamicBuffer >& chunks )
{GUCEF_TRACE;

    chunks.clear();
    if ( 0 == count )
        return CDRNResourceTransferResult();
    if ( STATE_OPENING == GetState() && !WaitOpen( m_options.openTimeoutMs ) )
        return GetResult();
    bool reopened = false;
    return ReadWindow( firstChunk, count, chunks, reopened );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult
CDRNResourceHandle::ReadRange( UInt64 offset, UInt32 length, CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    data.SetDataSize( 0 );
    if ( STATE_OPENING == GetState() && !WaitOpen( m_options.openTimeoutMs ) )
        return GetResult();
    CDRNResourceInfo info = GetInfo();
    if ( offset > info.size || length > info.size - offset )
        return CDRNResourceTransferResult( DRN_RESSTATUS_OUT_OF_RANGE, "the window reaches beyond the end of the resource" );
    if ( 0 == length )
        return CDRNResourceTransferResult();

    UInt64 first = offset / DRN_RESOURCE_CHUNK_SIZE;
    UInt64 end = ( offset + length + DRN_RESOURCE_CHUNK_SIZE - 1 ) / DRN_RESOURCE_CHUNK_SIZE;
    if ( !data.SetBufferSize( length, false, true ) )
        return CDRNResourceTransferResult( DRN_RESSTATUS_TOO_LARGE, "the window does not fit in memory" );
    data.SetDataSize( length );
    GUCEF::vector< CORE::CDynamicBuffer > chunks;
    CDRNResourceTransferResult result = ReadChunks( first, (UInt32) ( end - first ), chunks );
    if ( !result.IsOk() )
    {
        data.SetDataSize( 0 );
        return result;
    }
    UInt32 written = 0;
    for ( UInt32 i=0; i<chunks.size(); ++i )
    {
        UInt64 chunkStart = ( first + i ) * DRN_RESOURCE_CHUNK_SIZE;
        UInt64 from = chunkStart > offset ? chunkStart : offset;
        UInt64 to = chunkStart + chunks[ i ].GetDataSize();
        if ( to > offset + length )
            to = offset + length;
        if ( to <= from )
            continue;
        memcpy( static_cast< UInt8* >( data.GetBufferPtr() ) + ( from - offset ),
                static_cast< const UInt8* >( chunks[ i ].GetConstBufferPtr() ) + ( from - chunkStart ), (size_t) ( to - from ) );
        written += (UInt32) ( to - from );
    }
    return written == length ? CDRNResourceTransferResult() : CDRNResourceTransferResult( DRN_RESSTATUS_PROTOCOL_ERROR, "the window did not arrive whole" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
