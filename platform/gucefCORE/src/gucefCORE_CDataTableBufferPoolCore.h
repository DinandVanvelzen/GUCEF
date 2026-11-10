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

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOLCORE_H
#define GUCEF_CORE_CDATATABLEBUFFERPOOLCORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

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

/**
 *  The shared state of a CDataTableBufferPool: the free lists and the counters.
 *
 *  Reference counted by the pool object and by every buffer and batch object made from it, so buffers and batches may
 *  outlive the pool object. Once the pool object is gone (Shutdown) returned objects are freed instead of pooled, and
 *  the core deletes itself when the last of them is gone.
 *
 *  Ordering of the release path: a buffer whose count drops to zero (acquire-release decrement, so every write of
 *  every holder happens before) is returned with ReturnBuffer, which takes the lock of its size class only to link it
 *  into the free list. The alive flag is read under that same lock and Shutdown clears it and then passes through every
 *  lock before it empties the lists, so a buffer is either linked before that pass (and then seen by the emptying) or
 *  freed by the returning thread.
 */
class CDataTableBufferPoolCore
{
    public:

    explicit CDataTableBufferPoolCore( UInt64 maxPooledBytes );

    void AddReference( void );

    /** Deletes the core with the last reference */
    void ReleaseReference( void );

    TDataTableStatus Acquire( UInt32 minBytes, CDataTableBufferPtr& buffer );

    TDataTableStatus AcquireBatch( CDataTableBatchPtr& batch );

    /** Called when the last reference of the buffer went */
    void ReturnBuffer( CDataTableBuffer* buffer );

    /** Called when the last reference of the batch went, after the batch released its members */
    void ReturnBatch( CDataTableBatch* batch );

    /** The pool object is gone: frees the pooled objects, later returns free theirs */
    void Shutdown( void );

    void SetMaxPooledBytes( UInt64 maxPooledBytes );
    UInt64 GetMaxPooledBytes( void ) const;
    void Trim( UInt64 keepBytes );
    void GetStats( CDataTableBufferPoolStats& stats ) const;

    /** The size class index for a request, SizeClassCount when it is too large */
    static UInt32 SizeClassFor( UInt32 minBytes );
    static UInt32 SizeClassBytes( UInt32 sizeClass );

    private:

    ~CDataTableBufferPoolCore();

    CDataTableBufferPoolCore( const CDataTableBufferPoolCore& src );                /**< not implemented */
    CDataTableBufferPoolCore& operator=( const CDataTableBufferPoolCore& src );     /**< not implemented */

    void FreeBuffer( CDataTableBuffer* buffer );
    void FreeBatch( CDataTableBatch* batch );

    struct SSizeClass
    {
        MT::CMutex lock;
        CDataTableBuffer* freeList;
        UInt32 freeCount;
    };

    SSizeClass m_classes[ CDataTableBufferPool::SizeClassCount ];
    MT::CMutex m_batchLock;
    CDataTableBatch* m_freeBatches;
    UInt32 m_freeBatchCount;

    MT::CTAtomic< UInt32 > m_refCount;
    MT::CTAtomic< UInt32 > m_alive;                 /**< set to 0 by Shutdown before it takes the locks, see the class comment */
    MT::CTAtomic< UInt64 > m_maxPooledBytes;
    MT::CTAtomic< UInt64 > m_bytesPooled;
    MT::CTAtomic< UInt64 > m_bytesInUse;
    MT::CTAtomic< UInt64 > m_acquires;
    MT::CTAtomic< UInt64 > m_misses;
    MT::CTAtomic< UInt64 > m_returns;
    MT::CTAtomic< UInt64 > m_frees;
    MT::CTAtomic< UInt64 > m_allocationFailures;
    MT::CTAtomic< UInt64 > m_batchAcquires;
    MT::CTAtomic< UInt64 > m_batchMisses;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOLCORE_H ? */
