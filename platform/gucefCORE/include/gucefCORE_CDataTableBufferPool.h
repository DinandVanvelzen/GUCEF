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

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEBUFFER_H
#include "gucefCORE_CDataTableBuffer.h"
#define GUCEF_CORE_CDATATABLEBUFFER_H
#endif /* GUCEF_CORE_CDATATABLEBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDataTableBatchPtr;

/**
 *  Counters of a CDataTableBufferPool since it was made
 */
struct GUCEF_CORE_PUBLIC_CPP CDataTableBufferPoolStats
{
    UInt64 acquires;            /**< buffers handed out */
    UInt64 misses;              /**< buffers allocated on the heap because none of the size class was pooled */
    UInt64 returns;             /**< buffers whose last reference went */
    UInt64 frees;               /**< buffers freed: above maxPooledBytes, trimmed or at shut down */
    UInt64 allocationFailures;  /**< allocations that failed (OUT_OF_MEMORY) */
    UInt64 bytesPooled;         /**< bytes of buffers waiting in the pool */
    UInt64 bytesInUse;          /**< bytes of buffers handed out and not returned */
    UInt64 batchAcquires;       /**< batch objects handed out */
    UInt64 batchMisses;         /**< batch objects made on the heap because none was pooled */
    UInt32 batchesPooled;       /**< batch objects waiting in the pool */

    CDataTableBufferPoolStats( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Pool of CDataTableBuffer objects and of CDataTableBatch objects. Thread safe.
 *
 *  Size classes are powers of two from 4 KiB to 2 GiB. Acquire takes the class at or above minBytes + padding and
 *  reuses a pooled buffer of that class with its allocation; otherwise one is allocated: a miss. After a warm-up of
 *  the same shape, work makes no misses. One mutex per size class.
 *
 *  A buffer released while the pool already holds maxPooledBytes is freed instead of kept. Batch objects are pooled
 *  up to MaxPooledBatches, keeping the capacity of their members.
 *
 *  Buffers and batches may outlive the pool object: the pool's shared state lives until the last of them is gone,
 *  and buffers returned after the pool was destroyed are freed.
 *
 *  Debug builds (GUCEF_DEBUG_MODE) fill returned buffers with 0xDD; AddressSanitizer builds poison pooled buffers
 *  and unpoison them when they are handed out again, so a use after release fails loudly.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableBufferPool
{
    public:

    static const UInt32 MinSizeClassBytes = 4096;
    static const UInt32 SizeClassCount = 20;                /**< 4 KiB .. 2 GiB */
    static const UInt32 MaxPooledBatches = 1024;
    static const UInt64 DefaultMaxPooledBytes = 512ULL * 1024 * 1024;

    explicit CDataTableBufferPool( UInt64 maxPooledBytes = DefaultMaxPooledBytes );

    ~CDataTableBufferPool();

    /**
     *  A buffer with at least minBytes of capacity (plus DataTableBufferPadding), data size 0, padding zeroed.
     *  OUT_OF_MEMORY when the allocation fails, LIMIT_EXCEEDED above the largest size class.
     */
    TDataTableStatus Acquire( UInt32 minBytes, CDataTableBufferPtr& buffer );

    /**
     *  Makes sure the buffer has at least minBytes of capacity: when it has not (or is NULL) a buffer of a large enough
     *  size class is acquired, the data is copied over and the old buffer is released. The handle must be the only
     *  reference of a buffer that is still being produced. Doubling requests give geometric growth.
     */
    TDataTableStatus Reserve( CDataTableBufferPtr& buffer, UInt32 minBytes );

    /** An empty batch object: no rows, no columns, no schema, reference count 1 */
    TDataTableStatus AcquireBatch( CDataTableBatchPtr& batch );

    void SetMaxPooledBytes( UInt64 maxPooledBytes );

    UInt64 GetMaxPooledBytes( void ) const;

    /** Frees pooled (unused) buffers down to keepBytes, smallest size classes kept first; pooled batch objects too when keepBytes is 0 */
    void Trim( UInt64 keepBytes );

    /** Acquires, misses (heap allocations), returns, frees, bytes pooled, bytes in use */
    void GetStats( CDataTableBufferPoolStats& stats ) const;

    /** The allocation size of the size class Acquire uses for minBytes, 0 above the largest */
    static UInt32 GetSizeClassBytes( UInt32 minBytes );

    /**
     *  The number of shared pool states alive in the process. A state lives as long as its pool object or any buffer or
     *  batch made by it, so the count returns to its earlier value when all of them are gone: a diagnostic for leak checks.
     */
    static UInt32 GetLiveCoreCount( void );

    private:

    CDataTableBufferPool( const CDataTableBufferPool& src );                /**< not implemented */
    CDataTableBufferPool& operator=( const CDataTableBufferPool& src );     /**< not implemented */

    CDataTableBufferPoolCore* m_core;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */
