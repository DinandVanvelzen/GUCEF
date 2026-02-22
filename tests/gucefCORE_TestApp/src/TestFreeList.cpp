/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFREELIST_H
#include "gucefCORE_CTFreeList.h"
#define GUCEF_CORE_CTFREELIST_H
#endif /* GUCEF_CORE_CTFREELIST_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFreeList.h"

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
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// Lock type: use noLock for single-thread basic tests
typedef MT::CNoLock TNoLock;

// A simple test type with optional Clear()
struct TestObj
{
    Int32 value;
    TestObj() : value( 0 ) {}
    TestObj( const TestObj& src ) : value( src.value ) {}
    void Clear() { value = 0; }
};

typedef CTFreeList< TestObj, TNoLock >::TSharedPtr TestObjPtr;

// A test type with optional Clear() and its own shared pointer creator
// having said creator should force a different code path in the free list wrt which objects to maintain internally
struct TestObjWithPtrCreator : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >
{
    Int32 value;
    TestObjWithPtrCreator( void ) : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >( this ), value( 321 ) {}
    TestObjWithPtrCreator( const TestObjWithPtrCreator& src ) : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >( this ), value( src.value ) {}
    void Clear() { value = 0; }
};

typedef CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtr TestObjWithPtrCreatorPtr;

// A type without Clear()
struct TestObjNoClear
{
    Int32 value;
    TestObjNoClear( void ) : value( 123 ) {}
    TestObjNoClear( const TestObjNoClear& src ) : value( src.value ) {}
};

typedef CTFreeList< TestObjNoClear, TNoLock >::TSharedPtr TestObjNoClearPtr;

static void TestReserveAndAcquire_Reconstruct()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    ASSERT_TRUE( sizeof( CTFreeList< TestObj, TNoLock >::TSharedPtrCreator ) > sizeof( TestObj ) );

    pool.Reserve( 10 );
    UInt32 freed = pool.GetFreedCount();
    ASSERT_TRUE( freed >= 10 );

    TestObjPtr a = pool.Acquire();
    ASSERT_TRUE( !a.IsNULL() );
    ASSERT_TRUE( pool.GetActiveCount() == 1 );
    ASSERT_TRUE( pool.GetFreedCount() == freed-1 );

    a->value = 42;

    TestObj* rawPtr = static_cast< TestObj* >( a.GetPointerAlways() );
    CTFreeList< TestObj, TNoLock >::TSharedPtrCreator* sharedPtrCreator = static_cast< CTFreeList< TestObj, TNoLock >::TSharedPtrCreator* >( rawPtr );
    ASSERT_TRUE( sharedPtrCreator->value == 42 );

    TestObjPtr b = sharedPtrCreator->CreateBasicSharedPtr();
    ASSERT_TRUE( !b.IsNULL() );
    ASSERT_TRUE( b.GetPointerAlways() == a.GetPointerAlways() );
    ASSERT_TRUE( b->value == 42 );
    ASSERT_TRUE( b.GetReferenceCount() == 2 );
    a.Unlink();
    ASSERT_TRUE( b.GetReferenceCount() == 1 );

    // Dropping reference returns memory to pool
    b.Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == freed );
}

static void TestReserveBlocksAndAcquire()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    ASSERT_TRUE( pool.GetFreedCount() == 0 );
    UInt32 blockSize = pool.GetBlockSize();
    ASSERT_TRUE( blockSize > 0 );
    ASSERT_TRUE( pool.Reserve( 7 ) );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    GUCEF::vector< CTFreeList< TestObj, TNoLock >::TSharedPtr > objs;
    for ( Int32 i=0; i<7; ++i )
    {
        objs.push_back( pool.Acquire() );
        ASSERT_TRUE( !objs.back().IsNULL() );
        objs.back()->value = i;
    }
    ASSERT_TRUE( pool.GetActiveCount() == 7 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize-7 );

    // Release
    for ( Int32 i=0; i<7; ++i )
        objs[ i ].Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );
}

static void TestReserveBlocksAndAcquireWithPtrCreator()
{
    CTFreeList< TestObjWithPtrCreator, TNoLock > pool( CTFreeList< TestObjWithPtrCreator, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    ASSERT_TRUE( sizeof( CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtrCreator ) == sizeof( TestObjWithPtrCreator ) );

    ASSERT_TRUE( pool.GetFreedCount() == 0 );
    UInt32 blockSize = pool.GetBlockSize();
    ASSERT_TRUE( blockSize > 0 );
    ASSERT_TRUE( pool.Reserve( 7 ) );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    GUCEF::vector< CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtr > objs;
    for ( Int32 i=0; i<7; ++i )
    {
        objs.push_back( pool.Acquire() );
        ASSERT_TRUE( !objs.back().IsNULL() );
        objs.back()->value = i;
    }
    ASSERT_TRUE( pool.GetActiveCount() == 7 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize-7 );

    // Release
    for ( Int32 i=0; i<7; ++i )
        objs[ i ].Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );
}

static void TestClearPolicy()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );

    ASSERT_TRUE( pool.GetFreedCount() == 0 );
    pool.Reserve( 3 );
    UInt32 blockSize = pool.GetBlockSize();
    ASSERT_TRUE( blockSize > 0 );    
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    TestObjPtr a = pool.Acquire();
    a->value = 99;
    a.Unlink();
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    TestObjPtr b = pool.Acquire();
    ASSERT_TRUE( b->value == 0 );
}

static void TestNoClearPolicyGracefulFallback()
{
    // test to check if the container gracefully falls back to reconstruct when Clear() is not available
    // even when the Clear()-based-policy is requested. It says 'if available' after all.

    CTFreeList< TestObjNoClear, TNoLock > pool( CTFreeList< TestObjNoClear, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );

    pool.Reserve( 2 );
    UInt32 freeItems = pool.GetFreedCount();
    ASSERT_TRUE( freeItems >= 2 );

    TestObjNoClearPtr a = pool.Acquire();
    ASSERT_TRUE( a->value == 123 );
    a.Unlink();
    ASSERT_TRUE( pool.GetFreedCount() == freeItems );
}

static void TestDormantTransitions()
{
    CTFreeList< TestObj, TNoLock > pool;
    CTFreeList< TestObj, TNoLock >::TSharedPtr a = pool.Acquire();
    UInt32 freeItems = pool.GetFreedCount();
    ASSERT_TRUE( pool.GetActiveCount() == 1 );

    bool moved = pool.MarkDormant( a );
    ASSERT_TRUE( moved );
    ASSERT_TRUE( pool.GetDormantCount() == 1 );
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == freeItems );    

    bool back = pool.MarkActive( a );
    ASSERT_TRUE( back );
    ASSERT_TRUE( pool.GetActiveCount() == 1 );
    ASSERT_TRUE( pool.GetDormantCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == freeItems );

    a.Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() > freeItems );
}

}; /* namespace CORE */
}; /* namespace GUCEF */

void PerformFreeListTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "COMMENCING CTFreeList TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CTFreeList" );

    GUCEF_TESTFW_TESTCASE( "Test 1: Reserve and Acquire with Reconstruct" )
        try
        {
            GUCEF::CORE::TestReserveAndAcquire_Reconstruct();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 2: Reserve Blocks and Acquire" )
        try
        {
            GUCEF::CORE::TestReserveBlocksAndAcquire();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 3: Reserve Blocks and Acquire With PtrCreator" )
        try
        {
            GUCEF::CORE::TestReserveBlocksAndAcquireWithPtrCreator();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 4: Clear Policy" )
        try
        {
            GUCEF::CORE::TestClearPolicy();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 5: No Clear Policy Graceful Fallback" )
        try
        {
            GUCEF::CORE::TestNoClearPolicyGracefulFallback();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 6: Dormant Transitions" )
        try
        {
            GUCEF::CORE::TestDormantTransitions();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "ALL CTFreeList TESTS COMPLETED" );
}


