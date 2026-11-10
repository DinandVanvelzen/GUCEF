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

#ifndef GUCEF_CORE_CDATATABLEBATCHPTR_H
#define GUCEF_CORE_CDATATABLEBATCHPTR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

class CDataTableBatch;

/**
 *  Intrusive handle of a pooled CDataTableBatch (section 3.8 of the data tables design).
 *
 *  Copying adds a reference (an atomic increment), destroying or resetting releases one; no allocation is made per
 *  handle. When the last handle goes the batch releases its buffers and returns to the free list of its pool, on any
 *  thread. A handle object is not to be used by two threads at once; copies of it may be.
 *
 *  Declared apart from CDataTableBatch so column batches can hold a dictionary batch; include
 *  gucefCORE_CDataTableBatch.h to use it.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableBatchPtr
{
    public:

    CDataTableBatchPtr( void );

    CDataTableBatchPtr( const CDataTableBatchPtr& src );

    ~CDataTableBatchPtr();

    CDataTableBatchPtr& operator=( const CDataTableBatchPtr& src );

    inline const CDataTableBatch* operator->( void ) const { return m_batch; }

    inline const CDataTableBatch& operator*( void ) const { return *m_batch; }

    inline const CDataTableBatch* GetPointer( void ) const { return m_batch; }

    inline bool IsNULL( void ) const { return GUCEF_NULL == m_batch; }

    inline bool operator==( const CDataTableBatchPtr& other ) const { return m_batch == other.m_batch; }

    inline bool operator!=( const CDataTableBatchPtr& other ) const { return m_batch != other.m_batch; }

    /** Releases the reference, the handle is NULL afterwards */
    void Reset( void );

    /** Exchanges the batches of two handles without touching the reference counts */
    void Swap( CDataTableBatchPtr& other );

    /**
     *  Mutable access for the producer of the batch (a builder or a reader) while it fills a batch it just acquired
     *  and has not shared. GUCEF_NULL for a NULL handle.
     */
    inline CDataTableBatch* GetWritable( void ) const { return m_batch; }

    private:
    friend class CDataTableBufferPoolCore;

    /** Takes over one reference the caller holds */
    void Adopt( CDataTableBatch* batch );

    CDataTableBatch* m_batch;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEBATCHPTR_H ? */
