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

#ifndef GUCEF_CORE_CDATATABLEBUFFER_H
#define GUCEF_CORE_CDATATABLEBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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

class CDataTableBufferPoolCore;

/**
 *  A pooled, reference counted block of memory that column batches point into: values, validity, offsets, bytes,
 *  indices, selections, file chunks and decompressed pages.
 *
 *  The allocation is a CDynamicBuffer of a power of two size class of the pool. It is never reallocated, so pointers
 *  into it stay valid while a reference is held. At least DataTableBufferPadding zero filled bytes follow the data,
 *  so kernels may read past the last value without leaving the allocation.
 *
 *  Reached only through CDataTableBufferPtr. When the last reference goes the buffer returns to its pool, on any
 *  thread. A producer fills it before sharing it; once shared it is read only.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableBuffer
{
    public:

    /** The data: GetCapacity() bytes are writable, followed by DataTableBufferPadding readable bytes */
    inline void* GetData( void ) { return m_data; }
    inline const void* GetData( void ) const { return m_data; }

    /** Bytes of data the producer set */
    inline UInt32 GetDataSize( void ) const { return m_dataSize; }

    /**
     *  Sets how many bytes are data and zero fills the DataTableBufferPadding bytes after them.
     *  False when the size is above GetCapacity(); the data size is unchanged then.
     */
    bool SetDataSize( UInt32 dataSize );

    /** Bytes available for data: the size class minus DataTableBufferPadding */
    inline UInt32 GetCapacity( void ) const { return m_capacity; }

    /** Bytes of the allocation: the size class */
    UInt32 GetAllocationSize( void ) const;

    /** A snapshot of the reference count, for tests and diagnostics */
    UInt32 GetReferenceCount( void ) const;

    private:
    friend class CDataTableBufferPtr;
    friend class CDataTableBufferPoolCore;

    CDataTableBuffer( CDataTableBufferPoolCore* core, UInt32 sizeClass );
    ~CDataTableBuffer();

    CDataTableBuffer( const CDataTableBuffer& src );                /**< not implemented */
    CDataTableBuffer& operator=( const CDataTableBuffer& src );     /**< not implemented */

    void AddReference( void );
    void ReleaseReference( void );

    CDynamicBuffer m_buffer;                    /**< owns the allocation, its logical size is not used */
    void* m_data;
    UInt32 m_dataSize;
    UInt32 m_capacity;
    UInt32 m_sizeClass;
    MT::CTAtomic< UInt32 > m_refCount;
    CDataTableBufferPoolCore* m_core;
    CDataTableBuffer* m_nextFree;               /**< free list link while pooled */
};

/*-------------------------------------------------------------------------*/

/**
 *  Intrusive handle of a CDataTableBuffer. Copying adds a reference (an atomic increment), destroying or resetting
 *  releases one; no allocation is made per handle. A handle may be copied and released on any thread; one handle
 *  object is not to be used by two threads at once.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableBufferPtr
{
    public:

    CDataTableBufferPtr( void );

    CDataTableBufferPtr( const CDataTableBufferPtr& src );

    ~CDataTableBufferPtr();

    CDataTableBufferPtr& operator=( const CDataTableBufferPtr& src );

    inline CDataTableBuffer* operator->( void ) const { return m_buffer; }

    inline CDataTableBuffer& operator*( void ) const { return *m_buffer; }

    inline CDataTableBuffer* GetPointer( void ) const { return m_buffer; }

    inline bool IsNULL( void ) const { return GUCEF_NULL == m_buffer; }

    inline bool operator==( const CDataTableBufferPtr& other ) const { return m_buffer == other.m_buffer; }

    inline bool operator!=( const CDataTableBufferPtr& other ) const { return m_buffer != other.m_buffer; }

    /** Releases the reference, the handle is NULL afterwards */
    void Reset( void );

    /** Exchanges the buffers of two handles without touching the reference counts */
    void Swap( CDataTableBufferPtr& other );

    private:
    friend class CDataTableBufferPoolCore;

    /** Takes over one reference the caller holds */
    void Adopt( CDataTableBuffer* buffer );

    CDataTableBuffer* m_buffer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEBUFFER_H ? */
