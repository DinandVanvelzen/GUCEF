/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#if defined( __SANITIZE_ADDRESS__ )
  #define TESTDATATABLE_ASAN
#elif defined( __has_feature )
  #if __has_feature( address_sanitizer )
    #define TESTDATATABLE_ASAN
  #endif
#endif
#ifdef TESTDATATABLE_ASAN
  #include <sanitizer/asan_interface.h>
#endif

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestDataTableSupport.h"
#include "TestDataTableBufferPool.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------*/

static bool
PaddingIsZero( const CDataTableBuffer& buffer )
{
    const UInt8* padding = static_cast< const UInt8* >( buffer.GetData() ) + buffer.GetDataSize();
    for ( UInt32 i=0; i<DataTableBufferPadding; ++i )
    {
        if ( 0 != padding[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Copies, releases and acquires buffer and batch handles in a loop on a thread of its own, all on objects shared
 *  with other threads: the shared buffer through its own handle, and the pool.
 */
class CHandleStressThread : public MT::CActiveObject
{
    public:

    CHandleStressThread( const CDataTableBufferPtr& sharedBuffer, CDataTableBufferPool& pool, UInt32 iterations )
        : MT::CActiveObject()
        , m_shared( sharedBuffer )
        , m_pool( pool )
        , m_iterations( iterations )
        , failures( 0 )
        , done( 0 )
    {
    }

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 sharedValue = *static_cast< const UInt32* >( m_shared->GetData() );
        for ( UInt32 i=0; i<m_iterations; ++i )
        {
            // copies of the shared buffer handle
            CDataTableBufferPtr a( m_shared );
            CDataTableBufferPtr b;
            b = a;
            if ( *static_cast< const UInt32* >( b->GetData() ) != sharedValue )
                failures.FetchAdd( 1 );
            a.Reset();

            // a buffer of its own, written and released
            CDataTableBufferPtr own;
            if ( DATATABLE_STATUS_OK != m_pool.Acquire( 100 + ( i % 3 ) * 5000, own ) )
            {
                failures.FetchAdd( 1 );
                continue;
            }
            memset( own->GetData(), (int) ( i & 0xFF ), 100 );
            own->SetDataSize( 100 );

            // a batch pinning the shared buffer and its own, released on this thread or kept by another handle
            CDataTableBatchPtr batch;
            if ( DATATABLE_STATUS_OK != m_pool.AcquireBatch( batch ) )
            {
                failures.FetchAdd( 1 );
                continue;
            }
            CDataTableBatch* writable = batch.GetWritable();
            writable->SetColumnCount( 2 );
            writable->SetRowCount( 1 );
            writable->GetMutableColumn( 0 ).SetType( DATATABLE_TYPE_UINT32 );
            writable->GetMutableColumn( 0 ).SetFixedValues( 1, m_shared->GetData(), m_shared );
            writable->GetMutableColumn( 1 ).SetType( DATATABLE_TYPE_UINT8 );
            writable->GetMutableColumn( 1 ).SetFixedValues( 1, own->GetData(), own );
            own.Reset();
            CDataTableBatchPtr second( batch );
            batch.Reset();
            CVariant value;
            if ( !second->GetValue( 0, 0, value ) || value.AsUInt32() != sharedValue )
                failures.FetchAdd( 1 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CDataTableBufferPtr m_shared;
    CDataTableBufferPool& m_pool;
    UInt32 m_iterations;
    MT::CTAtomic< UInt32 > failures;
    MT::CTAtomic< UInt32 > done;
};

/*-------------------------------------------------------------------------*/

/** Releases the handles it was given on a thread of its own */
class CReleaseThread : public MT::CActiveObject
{
    public:

    CReleaseThread( const CDataTableBufferPtr& buffer, const CDataTableBatchPtr& batch )
        : MT::CActiveObject()
        , m_buffer( buffer )
        , m_batch( batch )
        , done( 0 )
    {
    }

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_buffer.Reset();
        m_batch.Reset();
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CDataTableBufferPtr m_buffer;
    CDataTableBatchPtr m_batch;
    MT::CTAtomic< UInt32 > done;
};

/*-------------------------------------------------------------------------*/

/**
 *  Waits for a signal and then releases the buffers and batches it holds: many threads do that at once while the
 *  main thread destroys the pool that made them.
 */
class CBurstReleaseThread : public MT::CActiveObject
{
    public:

    CBurstReleaseThread( MT::CTAtomic< UInt32 >& generation, MT::CTAtomic< UInt32 >& released, MT::CTAtomic< UInt32 >& stop )
        : MT::CActiveObject()
        , m_generation( generation )
        , m_released( released )
        , m_stop( stop )
        , m_seen( 0 )
        , buffers()
        , batches()
    {
    }

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        // spins for the signal of each round: the release has to start as soon as the pool object is destroyed
        while ( 0 == m_stop.Load() )
        {
            if ( m_generation.Load() == m_seen )
                continue;
            ++m_seen;
            for ( size_t i=0; i<buffers.size(); ++i )
                buffers[ i ].Reset();
            for ( size_t i=0; i<batches.size(); ++i )
                batches[ i ].Reset();
            buffers.clear();
            batches.clear();
            m_released.FetchAdd( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    GUCEF::vector< CDataTableBufferPtr > buffers;
    GUCEF::vector< CDataTableBatchPtr > batches;

    private:

    MT::CTAtomic< UInt32 >& m_generation;
    MT::CTAtomic< UInt32 >& m_released;
    MT::CTAtomic< UInt32 >& m_stop;
    UInt32 m_seen;
};

/*-------------------------------------------------------------------------*/

static void
WaitForDone( MT::CTAtomic< UInt32 >& done, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( 0 == done.Load() && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 5 );
        waited += 5;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformDataTableBufferPoolTests( void )
{
    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING data table buffer pool TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DataTableBufferPool" );

    GUCEF_TESTFW_TESTCASE( "Test: size classes are powers of two from 4 KiB to 2 GiB with room for the padding" )
        try
        {
            ASSERT_TRUE( 4096 == CDataTableBufferPool::GetSizeClassBytes( 0 ) );
            ASSERT_TRUE( 4096 == CDataTableBufferPool::GetSizeClassBytes( 4096 - DataTableBufferPadding ) );
            ASSERT_TRUE( 8192 == CDataTableBufferPool::GetSizeClassBytes( 4096 - DataTableBufferPadding + 1 ) );
            ASSERT_TRUE( 131072 == CDataTableBufferPool::GetSizeClassBytes( 16384 * 8 - DataTableBufferPadding ) );
            ASSERT_TRUE( 262144 == CDataTableBufferPool::GetSizeClassBytes( 16384 * 8 ) );
            ASSERT_TRUE( 2147483648U == CDataTableBufferPool::GetSizeClassBytes( 2147483648U - DataTableBufferPadding ) );
            ASSERT_TRUE( 0 == CDataTableBufferPool::GetSizeClassBytes( 2147483648U - DataTableBufferPadding + 1 ) );
            ASSERT_TRUE( 0 == CDataTableBufferPool::GetSizeClassBytes( GUCEF_UINT32MAX ) );

            CDataTableBufferPool pool;
            CDataTableBufferPtr buffer;
            ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == pool.Acquire( GUCEF_UINT32MAX, buffer ) && buffer.IsNULL() );
            ASSERT_TRUE( CDataTableBufferPool::DefaultMaxPooledBytes == pool.GetMaxPooledBytes() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: an acquired buffer: capacity, empty data, zeroed padding, one reference" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBufferPtr buffer;
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Acquire( 5000, buffer ) && !buffer.IsNULL() );
            ASSERT_TRUE( buffer->GetCapacity() >= 5000 && 8192 == buffer->GetAllocationSize() );
            ASSERT_TRUE( 8192 - DataTableBufferPadding == buffer->GetCapacity() );
            ASSERT_TRUE( 0 == buffer->GetDataSize() && 1 == buffer->GetReferenceCount() );
            ASSERT_TRUE( PaddingIsZero( *buffer ) );
            ASSERT_TRUE( 0 == ( (size_t) buffer->GetData() % 8 ) );

            // data then padding: setting the size zero fills the padding after the data
            memset( buffer->GetData(), 0xAB, buffer->GetCapacity() + DataTableBufferPadding );
            ASSERT_TRUE( buffer->SetDataSize( 300 ) && 300 == buffer->GetDataSize() && PaddingIsZero( *buffer ) );
            ASSERT_TRUE( 0xAB == static_cast< const UInt8* >( buffer->GetData() )[ 299 ] );
            ASSERT_TRUE( buffer->SetDataSize( buffer->GetCapacity() ) && PaddingIsZero( *buffer ) );
            ASSERT_FALSE( buffer->SetDataSize( buffer->GetCapacity() + 1 ) );
            ASSERT_TRUE( buffer->GetCapacity() == buffer->GetDataSize() );

            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Acquire( 0, buffer ) && 4096 == buffer->GetAllocationSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: handles count references and the buffer returns to the pool with the last one" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBufferPtr a;
            pool.Acquire( 10, a );
            CDataTableBuffer* raw = a.GetPointer();
            {
                CDataTableBufferPtr b( a );
                ASSERT_TRUE( 2 == raw->GetReferenceCount() && b == a );
                CDataTableBufferPtr c;
                c = b;
                ASSERT_TRUE( 3 == raw->GetReferenceCount() );
                c = c;
                ASSERT_TRUE( 3 == raw->GetReferenceCount() );
                CDataTableBufferPtr d;
                d.Swap( c );
                ASSERT_TRUE( c.IsNULL() && d.GetPointer() == raw && 3 == raw->GetReferenceCount() );
                d.Reset();
                ASSERT_TRUE( d.IsNULL() && 2 == raw->GetReferenceCount() );
            }
            ASSERT_TRUE( 1 == raw->GetReferenceCount() );

            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            ASSERT_TRUE( 1 == stats.acquires && 1 == stats.misses && 0 == stats.returns && 4096 == stats.bytesInUse && 0 == stats.bytesPooled );

            CDataTableBufferPtr other;
            pool.Acquire( 10, other );
            other = a;                                  // the other buffer goes back by assignment
            pool.GetStats( stats );
            ASSERT_TRUE( 1 == stats.returns && 4096 == stats.bytesPooled && 2 == raw->GetReferenceCount() );
            a.Reset();
            other.Reset();
            pool.GetStats( stats );
            ASSERT_TRUE( 2 == stats.returns && 0 == stats.bytesInUse && 8192 == stats.bytesPooled && 0 == stats.frees );

            // reuse: the last returned buffer of the class comes back, without a miss
            CDataTableBufferPtr again;
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Acquire( 100, again ) );
            ASSERT_TRUE( again.GetPointer() == raw && 1 == raw->GetReferenceCount() && 0 == raw->GetDataSize() );
            ASSERT_TRUE( PaddingIsZero( *again ) );
            pool.GetStats( stats );
            ASSERT_TRUE( 3 == stats.acquires && 2 == stats.misses && 4096 == stats.bytesPooled );

            // another size class is a miss
            CDataTableBufferPtr larger;
            pool.Acquire( 100000, larger );
            pool.GetStats( stats );
            ASSERT_TRUE( 3 == stats.misses && larger.GetPointer() != raw );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Reserve grows a buffer by size class and keeps its data" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBufferPtr buffer;
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Reserve( buffer, 1000 ) && !buffer.IsNULL() && buffer->GetCapacity() >= 1000 );
            for ( UInt32 i=0; i<1000; ++i )
                static_cast< UInt8* >( buffer->GetData() )[ i ] = (UInt8) ( i * 7 );
            buffer->SetDataSize( 1000 );
            CDataTableBuffer* first = buffer.GetPointer();
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Reserve( buffer, 2000 ) && buffer.GetPointer() == first );
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Reserve( buffer, 50000 ) && buffer.GetPointer() != first );
            ASSERT_TRUE( 65536 == buffer->GetAllocationSize() && 1000 == buffer->GetDataSize() && PaddingIsZero( *buffer ) );
            bool same = true;
            for ( UInt32 i=0; i<1000; ++i )
                same = same && static_cast< const UInt8* >( buffer->GetData() )[ i ] == (UInt8) ( i * 7 );
            ASSERT_TRUE( same );

            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            ASSERT_TRUE( 1 == stats.returns && 4096 == stats.bytesPooled );
            ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == pool.Reserve( buffer, GUCEF_UINT32MAX ) && buffer->GetDataSize() == 1000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the pooled byte budget, trimming and the statistics" )
        try
        {
            CDataTableBufferPool pool( 8192 );
            CDataTableBufferPtr a, b, c;
            pool.Acquire( 10, a );
            pool.Acquire( 10, b );
            pool.Acquire( 10, c );
            a.Reset();
            b.Reset();
            c.Reset();
            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            ASSERT_TRUE( 3 == stats.returns && 8192 == stats.bytesPooled && 1 == stats.frees );

            pool.Trim( 4096 );
            pool.GetStats( stats );
            ASSERT_TRUE( 4096 == stats.bytesPooled && 2 == stats.frees );
            pool.SetMaxPooledBytes( 0 );
            pool.GetStats( stats );
            ASSERT_TRUE( 0 == stats.bytesPooled && 3 == stats.frees && 0 == pool.GetMaxPooledBytes() );
            pool.Acquire( 10, a );
            a.Reset();
            pool.GetStats( stats );
            ASSERT_TRUE( 0 == stats.bytesPooled && 4 == stats.frees && 4 == stats.misses );

            pool.SetMaxPooledBytes( CDataTableBufferPool::DefaultMaxPooledBytes );
            pool.Acquire( 100000, a );
            pool.Acquire( 10, b );
            a.Reset();
            b.Reset();
            pool.Trim( 4096 );                          // the large class goes first
            pool.GetStats( stats );
            ASSERT_TRUE( 4096 == stats.bytesPooled );
            pool.Trim( 0 );
            pool.GetStats( stats );
            ASSERT_TRUE( 0 == stats.bytesPooled && stats.acquires == stats.returns && 0 == stats.bytesInUse && 0 == stats.allocationFailures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: batch objects come from the free list of the pool and release what they pin" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBatchPtr batch;
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.AcquireBatch( batch ) && !batch.IsNULL() );
            ASSERT_TRUE( 1 == batch->GetReferenceCount() && 0 == batch->GetColumnCount() && 0 == batch->GetRowCount() );
            ASSERT_TRUE( batch->GetSchema().IsNULL() && GUCEF_NULL == batch->GetSelection() );
            const CDataTableBatch* raw = batch.GetPointer();

            CDataTableBufferPtr values;
            pool.Acquire( 16, values );
            static_cast< Int32* >( values->GetData() )[ 0 ] = 42;
            static_cast< Int32* >( values->GetData() )[ 1 ] = -7;
            values->SetDataSize( 8 );
            CDataTableBatch* writable = batch.GetWritable();
            ASSERT_TRUE( writable->SetColumnCount( 1 ) );
            writable->SetRowCount( 2 );
            writable->SetSchemaColumnIndex( 0, 3 );
            writable->GetMutableColumn( 0 ).SetType( DATATABLE_TYPE_INT32 );
            writable->GetMutableColumn( 0 ).SetFixedValues( 2, values->GetData(), values );
            CDataTableBufferPtr selection;
            pool.Acquire( 4, selection );
            static_cast< UInt32* >( selection->GetData() )[ 0 ] = 1;
            writable->SetSelection( static_cast< const UInt32* >( selection->GetData() ), 1, selection );
            CDataTableBatchOrigin origin;
            origin.fileOrdinal = 2;
            origin.rowGroup = 5;
            origin.firstRowInRowGroup = 16384;
            writable->SetOrigin( origin );
            writable->SetSchema( CORETEST::MakeAllTypesSchema() );

            ASSERT_TRUE( 2 == values->GetReferenceCount() && 2 == selection->GetReferenceCount() );
            ASSERT_TRUE( 3 == batch->GetSchemaColumnIndex( 0 ) && GUCEF_UINT32MAX == batch->GetSchemaColumnIndex( 1 ) );
            ASSERT_TRUE( 1 == batch->GetSelectedRowCount() && 1 == batch->GetSelection()[ 0 ] && batch->GetOrigin() == origin );
            ASSERT_TRUE( 0 == batch->GetColumn( 5 ).GetRowCount() && DATATABLE_STORAGE_NONE == batch->GetColumn( 5 ).GetStorage() );
            CVariant value;
            ASSERT_TRUE( batch->GetValue( 1, 0, value ) && -7 == value.AsInt32() );
            ASSERT_FALSE( batch->GetValue( 2, 0, value ) || batch->GetValue( 0, 1, value ) );

            // the last handle releases the members and pools the object
            CDataTableBatchPtr copy( batch );
            ASSERT_TRUE( 2 == batch->GetReferenceCount() );
            batch.Reset();
            ASSERT_TRUE( 2 == values->GetReferenceCount() );
            copy.Reset();
            ASSERT_TRUE( 1 == values->GetReferenceCount() && 1 == selection->GetReferenceCount() );
            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            ASSERT_TRUE( 1 == stats.batchAcquires && 1 == stats.batchMisses && 1 == stats.batchesPooled );

            CDataTableBatchPtr reused;
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.AcquireBatch( reused ) && reused.GetPointer() == raw );
            ASSERT_TRUE( 0 == reused->GetColumnCount() && 0 == reused->GetRowCount() && GUCEF_NULL == reused->GetSelection() );
            ASSERT_TRUE( reused->GetSchema().IsNULL() && reused->GetOrigin() == CDataTableBatchOrigin() );
            pool.GetStats( stats );
            ASSERT_TRUE( 2 == stats.batchAcquires && 1 == stats.batchMisses && 0 == stats.batchesPooled );

            // fewer columns release the ones beyond
            CDataTableBatch* second = reused.GetWritable();
            second->SetColumnCount( 2 );
            second->GetMutableColumn( 1 ).SetType( DATATABLE_TYPE_INT32 );
            second->GetMutableColumn( 1 ).SetFixedValues( 2, values->GetData(), values );
            ASSERT_TRUE( 2 == values->GetReferenceCount() );
            second->SetColumnCount( 1 );
            ASSERT_TRUE( 1 == values->GetReferenceCount() && 1 == reused->GetColumnCount() );
            reused.Reset();
            pool.Trim( 0 );
            pool.GetStats( stats );
            ASSERT_TRUE( 0 == stats.batchesPooled );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: buffers and batches released on another thread return to the pool" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBufferPtr buffer;
            CDataTableBatchPtr batch;
            pool.Acquire( 10, buffer );
            pool.AcquireBatch( batch );
            CReleaseThread* thread = GUCEF_NEW CReleaseThread( buffer, batch );
            buffer.Reset();
            batch.Reset();
            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            ASSERT_TRUE( 0 == stats.returns && 0 == stats.batchesPooled );
            ASSERT_TRUE( thread->Activate() );
            WaitForDone( thread->done, 10000 );
            thread->Deactivate( false, true );
            pool.GetStats( stats );
            ASSERT_TRUE( 1 == thread->done.Load() && 1 == stats.returns && 1 == stats.batchesPooled && 0 == stats.bytesInUse );
            GUCEF_DELETE thread;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: reference counts stay exact under concurrent copies, releases and acquires on many threads" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBufferPtr shared;
            pool.Acquire( 64, shared );
            *static_cast< UInt32* >( shared->GetData() ) = 0xC0FFEE;
            shared->SetDataSize( 4 );

            const UInt32 threadCount = 8;
            const UInt32 iterations = 2000;
            CHandleStressThread* threads[ threadCount ];
            for ( UInt32 i=0; i<threadCount; ++i )
                threads[ i ] = GUCEF_NEW CHandleStressThread( shared, pool, iterations );
            ASSERT_TRUE( 1 + threadCount == shared->GetReferenceCount() );
            for ( UInt32 i=0; i<threadCount; ++i )
                ASSERT_TRUE( threads[ i ]->Activate() );

            // the main thread copies and releases the shared buffer at the same time
            for ( UInt32 i=0; i<iterations * 4; ++i )
            {
                CDataTableBufferPtr copy( shared );
                CDataTableBufferPtr another = copy;
            }

            UInt32 failures = 0;
            for ( UInt32 i=0; i<threadCount; ++i )
            {
                WaitForDone( threads[ i ]->done, 120000 );
                threads[ i ]->Deactivate( false, true );
                ASSERT_TRUE( 1 == threads[ i ]->done.Load() );
                failures += threads[ i ]->failures.Load();
                GUCEF_DELETE threads[ i ];
            }
            ASSERT_TRUE( 0 == failures );
            ASSERT_TRUE( 1 == shared->GetReferenceCount() );

            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            ASSERT_TRUE( stats.acquires == stats.returns + 1 );          // only the shared buffer is held
            ASSERT_TRUE( shared->GetAllocationSize() == stats.bytesInUse );
            ASSERT_TRUE( stats.batchAcquires == (UInt64) threadCount * iterations );
            ASSERT_TRUE( stats.misses <= 4 * threadCount );              // every thread holds at most a few buffers at once
            ASSERT_TRUE( stats.batchMisses <= threadCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: buffers and batches outlive the pool object that made them" )
        try
        {
            CDataTableBufferPool* pool = GUCEF_NEW CDataTableBufferPool();
            CDataTableBufferPtr buffer;
            CDataTableBatchPtr batch;
            CDataTableBufferPtr pooled;
            pool->Acquire( 1000, buffer );
            pool->Acquire( 1000, pooled );
            pool->AcquireBatch( batch );
            pooled.Reset();                              // waits in the pool, freed with it
            batch.GetWritable()->SetColumnCount( 1 );
            batch.GetWritable()->GetMutableColumn( 0 ).SetType( DATATABLE_TYPE_UINT8 );
            batch.GetWritable()->GetMutableColumn( 0 ).SetFixedValues( 1, buffer->GetData(), buffer );
            batch.GetWritable()->SetRowCount( 1 );
            GUCEF_DELETE pool;
            pool = GUCEF_NULL;

            memset( buffer->GetData(), 5, 1000 );
            ASSERT_TRUE( buffer->SetDataSize( 1000 ) );
            CVariant value;
            ASSERT_TRUE( batch->GetValue( 0, 0, value ) && 5 == value.AsUInt8() );
            CDataTableBatchPtr copy( batch );
            batch.Reset();
            buffer.Reset();
            ASSERT_TRUE( copy->GetValue( 0, 0, value ) && 5 == value.AsUInt8() );
            copy.Reset();                                // the last object frees the shared state of the pool
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: buffers released on many threads while the pool object is destroyed are all freed, the shared state of the pool does not leak" )
        try
        {
            const UInt32 baseline = CDataTableBufferPool::GetLiveCoreCount();
            const UInt32 threadCount = 6;
            const CString roundsText = GUCEF_TESTFW_GETENV( "GUCEF_TEST_POOL_RACE_ROUNDS" ).c_str();
            const UInt32 rounds = roundsText.IsNULLOrEmpty() ? 20000 : StringToUInt32( roundsText );
            MT::CTAtomic< UInt32 > generation( 0 );
            MT::CTAtomic< UInt32 > released( 0 );
            MT::CTAtomic< UInt32 > stop( 0 );
            CBurstReleaseThread* threads[ threadCount ];
            for ( UInt32 t=0; t<threadCount; ++t )
            {
                threads[ t ] = GUCEF_NEW CBurstReleaseThread( generation, released, stop );
                ASSERT_TRUE( threads[ t ]->Activate() );
            }

            UInt32 leaked = 0;
            UInt32 state = 12345;
            for ( UInt32 round=0; round<rounds; ++round )
            {
                CDataTableBufferPool* pool = GUCEF_NEW CDataTableBufferPool();
                for ( UInt32 t=0; t<threadCount; ++t )
                {
                    for ( UInt32 b=0; b<6; ++b )
                    {
                        CDataTableBufferPtr buffer;
                        pool->Acquire( 100 + b * 5000, buffer );
                        threads[ t ]->buffers.push_back( buffer );
                    }
                    CDataTableBatchPtr batch;
                    pool->AcquireBatch( batch );
                    threads[ t ]->batches.push_back( batch );
                }
                released.Store( 0 );
                generation.FetchAdd( 1 );

                // the pool object goes while the threads are releasing: a little spin, a different one every round
                state = state * 1103515245u + 12345u;
                for ( volatile UInt32 spin = 0; spin < ( ( state >> 16 ) & 0xFF ); ++spin ) {}
                GUCEF_DELETE pool;

                const UInt64 waitStart = MT::PrecisionTickCount();
                while ( released.Load() < threadCount && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - waitStart ) < 60000 ) {}
                ASSERT_TRUE( released.Load() == threadCount );
                if ( CDataTableBufferPool::GetLiveCoreCount() != baseline )
                {
                    // every holder has let go: a state that is still alive is a leak, one per pool
                    ++leaked;
                    break;
                }
            }
            stop.Store( 1 );
            for ( UInt32 t=0; t<threadCount; ++t )
            {
                threads[ t ]->Deactivate( false, true );
                GUCEF_DELETE threads[ t ];
            }
            if ( leaked > 0 )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "A pool state outlived the pool and all of its buffers and batches" );
            ASSERT_TRUE( 0 == leaked );
            ASSERT_TRUE( baseline == CDataTableBufferPool::GetLiveCoreCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: released buffers are filled (debug builds) and poisoned (AddressSanitizer builds)" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBufferPtr buffer;
            pool.Acquire( 1000, buffer );
            memset( buffer->GetData(), 0x11, 1000 );
            void* data = buffer->GetData();
            buffer.Reset();

            #ifdef TESTDATATABLE_ASAN
            ASSERT_TRUE( 0 != __asan_address_is_poisoned( data ) );
            ASSERT_TRUE( 0 != __asan_address_is_poisoned( static_cast< char* >( data ) + 4095 ) );
            #endif

            pool.Acquire( 1000, buffer );
            ASSERT_TRUE( buffer->GetData() == data );
            #ifdef TESTDATATABLE_ASAN
            ASSERT_TRUE( 0 == __asan_address_is_poisoned( data ) );
            ASSERT_TRUE( 0 == __asan_address_is_poisoned( static_cast< char* >( data ) + 4095 ) );
            #endif
            #ifdef GUCEF_DEBUG_MODE
            ASSERT_TRUE( 0xDD == static_cast< const UInt8* >( buffer->GetData() )[ 500 ] );
            #endif
            ASSERT_TRUE( 0 == static_cast< const UInt8* >( buffer->GetData() )[ 0 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a warm pool hands out buffers and batches without a heap allocation" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            CDataTableBufferPool pool;
            const UInt32 sizes[] = { 10, 5000, 131072, 70000 };
            for ( UInt32 warm=0; warm<2; ++warm )
            {
                CDataTableBufferPtr held[ 4 ];
                for ( UInt32 i=0; i<4; ++i )
                    pool.Acquire( sizes[ i ], held[ i ] );
                CDataTableBatchPtr batch;
                pool.AcquireBatch( batch );
                batch.GetWritable()->SetColumnCount( 3 );
            }

            CDataTableBufferPoolStats before;
            pool.GetStats( before );
            CORETEST::StartAllocationCounting();
            for ( UInt32 round=0; round<100; ++round )
            {
                CDataTableBufferPtr held[ 4 ];
                for ( UInt32 i=0; i<4; ++i )
                    pool.Acquire( sizes[ i ], held[ i ] );
                CDataTableBatchPtr batch;
                pool.AcquireBatch( batch );
                CDataTableBatch* writable = batch.GetWritable();
                writable->SetColumnCount( 3 );
                writable->GetMutableColumn( 0 ).SetType( DATATABLE_TYPE_INT64 );
                writable->GetMutableColumn( 0 ).SetFixedValues( 1, held[ 1 ]->GetData(), held[ 1 ] );
                CDataTableBatchPtr copy( batch );
                held[ 0 ] = held[ 1 ];
            }
            UInt32 allocations = CORETEST::StopAllocationCounting();
            if ( allocations > 0 )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Warm pool allocated " << allocations << " times:\n" << CORETEST::GetCountedAllocationStacks() );
            ASSERT_TRUE( 0 == allocations );
            CDataTableBufferPoolStats after;
            pool.GetStats( after );
            ASSERT_TRUE( before.misses == after.misses && before.batchMisses == after.batchMisses );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the default pool of the core global" )
        try
        {
            CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
            CDataTableBufferPtr buffer;
            ASSERT_TRUE( DATATABLE_STATUS_OK == pool.Acquire( 100, buffer ) && !buffer.IsNULL() );
            ASSERT_TRUE( &pool == &CCoreGlobal::Instance()->GetDataTableBufferPool() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
