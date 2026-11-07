/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
#include <new>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CDataTableBufferPoolCore.h"

/*
 *  With AddressSanitizer pooled buffers are poisoned, so a read or write through a released span fails loudly
 */
#if defined( __SANITIZE_ADDRESS__ )
  #define GUCEF_CORE_DATATABLE_ASAN
#elif defined( __has_feature )
  #if __has_feature( address_sanitizer )
    #define GUCEF_CORE_DATATABLE_ASAN
  #endif
#endif

#ifdef GUCEF_CORE_DATATABLE_ASAN
  #include <sanitizer/asan_interface.h>
  #define GUCEF_DATATABLE_POISON( address, size )      ASAN_POISON_MEMORY_REGION( address, size )
  #define GUCEF_DATATABLE_UNPOISON( address, size )    ASAN_UNPOISON_MEMORY_REGION( address, size )
#else
  #define GUCEF_DATATABLE_POISON( address, size )      ( (void) ( address ), (void) ( size ) )
  #define GUCEF_DATATABLE_UNPOISON( address, size )    ( (void) ( address ), (void) ( size ) )
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableBufferPoolStats::CDataTableBufferPoolStats( void )
    : acquires( 0 )
    , misses( 0 )
    , returns( 0 )
    , frees( 0 )
    , allocationFailures( 0 )
    , bytesPooled( 0 )
    , bytesInUse( 0 )
    , batchAcquires( 0 )
    , batchMisses( 0 )
    , batchesPooled( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

namespace {

MT::CTAtomic< UInt32 >&
LiveCoreCounter( void )
{GUCEF_TRACE;

    static MT::CTAtomic< UInt32 > counter( 0 );
    return counter;
}

}

/*-------------------------------------------------------------------------*/

CDataTableBufferPoolCore::CDataTableBufferPoolCore( UInt64 maxPooledBytes )
    : m_freeBatches( GUCEF_NULL )
    , m_freeBatchCount( 0 )
    , m_refCount( 1 )
    , m_alive( 1 )
    , m_maxPooledBytes( maxPooledBytes )
    , m_bytesPooled( 0 )
    , m_bytesInUse( 0 )
    , m_acquires( 0 )
    , m_misses( 0 )
    , m_returns( 0 )
    , m_frees( 0 )
    , m_allocationFailures( 0 )
    , m_batchAcquires( 0 )
    , m_batchMisses( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<CDataTableBufferPool::SizeClassCount; ++i )
    {
        m_classes[ i ].freeList = GUCEF_NULL;
        m_classes[ i ].freeCount = 0;
    }
    LiveCoreCounter().FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

CDataTableBufferPoolCore::~CDataTableBufferPoolCore()
{GUCEF_TRACE;

    LiveCoreCounter().FetchSub( 1 );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::AddReference( void )
{GUCEF_TRACE;

    m_refCount.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::ReleaseReference( void )
{GUCEF_TRACE;

    if ( 1 == m_refCount.FetchSub( 1, MT::MEMORDER_ACQ_REL ) )
        GUCEF_DELETE this;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBufferPoolCore::SizeClassFor( UInt32 minBytes )
{GUCEF_TRACE;

    UInt64 required = (UInt64) minBytes + DataTableBufferPadding;
    UInt64 classBytes = CDataTableBufferPool::MinSizeClassBytes;
    for ( UInt32 i=0; i<CDataTableBufferPool::SizeClassCount; ++i )
    {
        if ( classBytes >= required )
            return i;
        classBytes <<= 1;
    }
    return CDataTableBufferPool::SizeClassCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBufferPoolCore::SizeClassBytes( UInt32 sizeClass )
{GUCEF_TRACE;

    return CDataTableBufferPool::MinSizeClassBytes << sizeClass;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBufferPoolCore::Acquire( UInt32 minBytes, CDataTableBufferPtr& buffer )
{GUCEF_TRACE;

    buffer.Reset();
    const UInt32 sizeClass = SizeClassFor( minBytes );
    if ( sizeClass >= CDataTableBufferPool::SizeClassCount )
        return DATATABLE_STATUS_LIMIT_EXCEEDED;
    const UInt32 classBytes = SizeClassBytes( sizeClass );

    CDataTableBuffer* pooled = GUCEF_NULL;
    {
        SSizeClass& sizeClassEntry = m_classes[ sizeClass ];
        MT::CScopeMutex lock( sizeClassEntry.lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        pooled = sizeClassEntry.freeList;
        if ( GUCEF_NULL != pooled )
        {
            sizeClassEntry.freeList = pooled->m_nextFree;
            --sizeClassEntry.freeCount;
        }
    }

    if ( GUCEF_NULL != pooled )
    {
        m_bytesPooled.FetchSub( classBytes, MT::MEMORDER_RELAXED );
        GUCEF_DATATABLE_UNPOISON( pooled->m_data, classBytes );
        pooled->m_nextFree = GUCEF_NULL;
    }
    else
    {
        // a miss: a new buffer of the size class
        try
        {
            pooled = GUCEF_NEW CDataTableBuffer( this, sizeClass );
        }
        catch ( const std::bad_alloc& )
        {
            pooled = GUCEF_NULL;
        }
        if ( GUCEF_NULL == pooled || GUCEF_NULL == pooled->m_data )
        {
            if ( GUCEF_NULL != pooled )
                GUCEF_DELETE pooled;
            m_allocationFailures.FetchAdd( 1, MT::MEMORDER_RELAXED );
            return DATATABLE_STATUS_OUT_OF_MEMORY;
        }
        AddReference();
        m_misses.FetchAdd( 1, MT::MEMORDER_RELAXED );
    }

    m_acquires.FetchAdd( 1, MT::MEMORDER_RELAXED );
    m_bytesInUse.FetchAdd( classBytes, MT::MEMORDER_RELAXED );

    pooled->m_refCount.Store( 1, MT::MEMORDER_RELAXED );
    pooled->m_dataSize = 0;
    memset( pooled->m_data, 0, DataTableBufferPadding );
    buffer.Adopt( pooled );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::ReturnBuffer( CDataTableBuffer* buffer )
{GUCEF_TRACE;

    const UInt32 classBytes = SizeClassBytes( buffer->m_sizeClass );
    m_returns.FetchAdd( 1, MT::MEMORDER_RELAXED );
    m_bytesInUse.FetchSub( classBytes, MT::MEMORDER_RELAXED );

    #ifdef GUCEF_DEBUG_MODE
    memset( buffer->m_data, 0xDD, classBytes );
    #endif

    // reserve room in the pooled byte budget first, so the bound holds under concurrent returns
    bool pool = false;
    const UInt64 pooledBefore = m_bytesPooled.FetchAdd( classBytes, MT::MEMORDER_RELAXED );
    if ( pooledBefore + classBytes <= m_maxPooledBytes.Load( MT::MEMORDER_RELAXED ) )
    {
        SSizeClass& sizeClassEntry = m_classes[ buffer->m_sizeClass ];
        MT::CScopeMutex lock( sizeClassEntry.lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        if ( 0 != m_alive.Load( MT::MEMORDER_RELAXED ) )
        {
            GUCEF_DATATABLE_POISON( buffer->m_data, classBytes );
            buffer->m_nextFree = sizeClassEntry.freeList;
            sizeClassEntry.freeList = buffer;
            ++sizeClassEntry.freeCount;
            pool = true;
        }
    }
    if ( !pool )
    {
        m_bytesPooled.FetchSub( classBytes, MT::MEMORDER_RELAXED );
        FreeBuffer( buffer );
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::FreeBuffer( CDataTableBuffer* buffer )
{GUCEF_TRACE;

    // the last statement may delete the core: nothing of it is used after
    m_frees.FetchAdd( 1, MT::MEMORDER_RELAXED );
    GUCEF_DATATABLE_UNPOISON( buffer->m_data, SizeClassBytes( buffer->m_sizeClass ) );
    GUCEF_DELETE buffer;
    ReleaseReference();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBufferPoolCore::AcquireBatch( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    batch.Reset();
    CDataTableBatch* pooled = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_batchLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        pooled = m_freeBatches;
        if ( GUCEF_NULL != pooled )
        {
            m_freeBatches = pooled->m_nextFree;
            --m_freeBatchCount;
        }
    }

    if ( GUCEF_NULL != pooled )
    {
        pooled->m_nextFree = GUCEF_NULL;
    }
    else
    {
        try
        {
            pooled = GUCEF_NEW CDataTableBatch( this );
        }
        catch ( const std::bad_alloc& )
        {
            m_allocationFailures.FetchAdd( 1, MT::MEMORDER_RELAXED );
            return DATATABLE_STATUS_OUT_OF_MEMORY;
        }
        AddReference();
        m_batchMisses.FetchAdd( 1, MT::MEMORDER_RELAXED );
    }

    m_batchAcquires.FetchAdd( 1, MT::MEMORDER_RELAXED );
    pooled->m_refCount.Store( 1, MT::MEMORDER_RELAXED );
    batch.Adopt( pooled );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::ReturnBatch( CDataTableBatch* batch )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_batchLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        if ( 0 != m_alive.Load( MT::MEMORDER_RELAXED ) && m_freeBatchCount < CDataTableBufferPool::MaxPooledBatches )
        {
            batch->m_nextFree = m_freeBatches;
            m_freeBatches = batch;
            ++m_freeBatchCount;
            return;
        }
    }
    FreeBatch( batch );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::FreeBatch( CDataTableBatch* batch )
{GUCEF_TRACE;

    // the last statement may delete the core: nothing of it is used after
    GUCEF_DELETE batch;
    ReleaseReference();
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::Shutdown( void )
{GUCEF_TRACE;

    // the core stays alive through this: the pool object's reference is released by the caller afterwards
    m_alive.Store( 0, MT::MEMORDER_RELEASE );

    // A returning thread adds to the pooled byte count first and then takes the lock of its size class to read the alive
    // flag. Trim() below only takes a lock while it sees pooled bytes, so on its own it could look before that addition
    // and miss a buffer that then reads the flag as still set. Passing through every lock after the flag was cleared
    // orders the two: a thread that takes a lock after this pass sees the flag cleared and frees its buffer, a thread that
    // took it before has made its addition visible to Trim() through the lock.
    for ( UInt32 i=0; i<CDataTableBufferPool::SizeClassCount; ++i )
    {
        MT::CScopeMutex lock( m_classes[ i ].lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    }
    Trim( 0 );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::SetMaxPooledBytes( UInt64 maxPooledBytes )
{GUCEF_TRACE;

    m_maxPooledBytes.Store( maxPooledBytes, MT::MEMORDER_RELAXED );
    if ( m_bytesPooled.Load( MT::MEMORDER_RELAXED ) > maxPooledBytes )
        Trim( maxPooledBytes );
}

/*-------------------------------------------------------------------------*/

UInt64
CDataTableBufferPoolCore::GetMaxPooledBytes( void ) const
{GUCEF_TRACE;

    return m_maxPooledBytes.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::Trim( UInt64 keepBytes )
{GUCEF_TRACE;

    // largest size classes go first, they free the most for the fewest buffers
    for ( Int32 c=(Int32) CDataTableBufferPool::SizeClassCount-1; c>=0; --c )
    {
        const UInt32 classBytes = SizeClassBytes( (UInt32) c );
        while ( m_bytesPooled.Load( MT::MEMORDER_RELAXED ) > keepBytes )
        {
            CDataTableBuffer* buffer = GUCEF_NULL;
            {
                SSizeClass& sizeClassEntry = m_classes[ c ];
                MT::CScopeMutex lock( sizeClassEntry.lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                buffer = sizeClassEntry.freeList;
                if ( GUCEF_NULL != buffer )
                {
                    sizeClassEntry.freeList = buffer->m_nextFree;
                    --sizeClassEntry.freeCount;
                }
            }
            if ( GUCEF_NULL == buffer )
                break;
            m_bytesPooled.FetchSub( classBytes, MT::MEMORDER_RELAXED );
            FreeBuffer( buffer );
        }
    }

    if ( 0 == keepBytes )
    {
        CDataTableBatch* batches = GUCEF_NULL;
        {
            MT::CScopeMutex lock( m_batchLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
            batches = m_freeBatches;
            m_freeBatches = GUCEF_NULL;
            m_freeBatchCount = 0;
        }
        while ( GUCEF_NULL != batches )
        {
            CDataTableBatch* next = batches->m_nextFree;
            FreeBatch( batches );
            batches = next;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPoolCore::GetStats( CDataTableBufferPoolStats& stats ) const
{GUCEF_TRACE;

    stats.acquires = m_acquires.Load( MT::MEMORDER_RELAXED );
    stats.misses = m_misses.Load( MT::MEMORDER_RELAXED );
    stats.returns = m_returns.Load( MT::MEMORDER_RELAXED );
    stats.frees = m_frees.Load( MT::MEMORDER_RELAXED );
    stats.allocationFailures = m_allocationFailures.Load( MT::MEMORDER_RELAXED );
    stats.bytesPooled = m_bytesPooled.Load( MT::MEMORDER_RELAXED );
    stats.bytesInUse = m_bytesInUse.Load( MT::MEMORDER_RELAXED );
    stats.batchAcquires = m_batchAcquires.Load( MT::MEMORDER_RELAXED );
    stats.batchMisses = m_batchMisses.Load( MT::MEMORDER_RELAXED );
    MT::CScopeMutex lock( m_batchLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    stats.batchesPooled = m_freeBatchCount;
}

/*-------------------------------------------------------------------------*/

CDataTableBufferPool::CDataTableBufferPool( UInt64 maxPooledBytes )
    : m_core( GUCEF_NEW CDataTableBufferPoolCore( maxPooledBytes ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableBufferPool::~CDataTableBufferPool()
{GUCEF_TRACE;

    CDataTableBufferPoolStats stats;
    m_core->GetStats( stats );
    if ( stats.bytesInUse > 0 )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataTableBufferPool: destroyed while " << stats.bytesInUse <<
            " bytes of buffers are held, they are freed when their last reference goes" );
    }
    m_core->Shutdown();
    m_core->ReleaseReference();
    m_core = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBufferPool::Acquire( UInt32 minBytes, CDataTableBufferPtr& buffer )
{GUCEF_TRACE;

    return m_core->Acquire( minBytes, buffer );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBufferPool::Reserve( CDataTableBufferPtr& buffer, UInt32 minBytes )
{GUCEF_TRACE;

    if ( !buffer.IsNULL() && buffer->GetCapacity() >= minBytes )
        return DATATABLE_STATUS_OK;

    CDataTableBufferPtr larger;
    TDataTableStatus status = m_core->Acquire( minBytes, larger );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    if ( !buffer.IsNULL() )
    {
        const UInt32 dataSize = buffer->GetDataSize();
        memcpy( larger->GetData(), buffer->GetData(), dataSize );
        larger->SetDataSize( dataSize );
    }
    buffer.Swap( larger );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBufferPool::AcquireBatch( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    return m_core->AcquireBatch( batch );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPool::SetMaxPooledBytes( UInt64 maxPooledBytes )
{GUCEF_TRACE;

    m_core->SetMaxPooledBytes( maxPooledBytes );
}

/*-------------------------------------------------------------------------*/

UInt64
CDataTableBufferPool::GetMaxPooledBytes( void ) const
{GUCEF_TRACE;

    return m_core->GetMaxPooledBytes();
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPool::Trim( UInt64 keepBytes )
{GUCEF_TRACE;

    m_core->Trim( keepBytes );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPool::GetStats( CDataTableBufferPoolStats& stats ) const
{GUCEF_TRACE;

    m_core->GetStats( stats );
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBufferPool::GetSizeClassBytes( UInt32 minBytes )
{GUCEF_TRACE;

    UInt32 sizeClass = CDataTableBufferPoolCore::SizeClassFor( minBytes );
    if ( sizeClass >= SizeClassCount )
        return 0;
    return CDataTableBufferPoolCore::SizeClassBytes( sizeClass );
}

UInt32
CDataTableBufferPool::GetLiveCoreCount( void )
{GUCEF_TRACE;

    return LiveCoreCounter().Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
